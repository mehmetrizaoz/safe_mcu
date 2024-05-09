#include "rpmsg/rpmsg_rtos.h"
#include "FreeRTOS.h"
#include "board.h"
#include "uart_imx.h"
#include "debug_console_imx.h"
#include "comm_skb.h"
#include <stdlib.h> 
#include "semphr.h"
#include "i2c.h"
#include "gpio.h"

struct remote_device *rdev = NULL;
struct rpmsg_channel *app_chnl = NULL;
static SemaphoreHandle_t xSemaphore;

uint8_t skb_rec_buf[MAX_BUF_SIZE];
uint8_t skb_rec_buf_wptr = 0;
uint8_t skb_rec_buf_rptr = 0;
uint8_t skb_rec_buf_data_count = 0;
bool is_rec_buf_full = false;

QueueHandle_t ack_queue_of_skb_received_data;
QueueHandle_t ack_queue_received_from_skb;
QueueHandle_t data_queue_receied_from_a7;

uint32_t skb_ack_timeout = 0;
bool waiting_skb_ack = false;
uint8_t sent_skb_packet_packet_id = 1;
bool wrong_ack_received_from_skb = false;

void pointPreviousPlace(uint8_t *ptr, uint8_t shift){
    for(int i=0; i<shift; i++){
        if(*ptr == 0){
            *ptr = MAX_BUF_SIZE - 1;
        }
        else{
            (*ptr)--;
        }
    }
}

void pointNextPlace(uint8_t *ptr, uint8_t shift){
    for(int i=0; i<shift; i++){
        if(*ptr == MAX_BUF_SIZE - 1){
            *ptr = 0;
        }
        else{
            (*ptr)++;
        }
    }
}

void skb_comm_semaphore_init(){
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xSemaphore);
    ack_queue_of_skb_received_data = xQueueCreate(256, sizeof(uint8_t));
    ack_queue_received_from_skb = xQueueCreate(256, sizeof(uint8_t));
    data_queue_receied_from_a7 = xQueueCreate(256, sizeof(uint8_t));
}

bool get_packet_from_a7_rx_buffer(uint8_t *packet_length, uint8_t **start_address){
    uint8_t *a7_packet_data;
    uint8_t a7Data;

    if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == errQUEUE_EMPTY){
        return false;
    }
    
    if(a7Data != SOF){ 
        return false;
    }

    if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == errQUEUE_EMPTY){
        return false;
    }

    if(a7Data != SOT){   
        return false;
    }               

    if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == errQUEUE_EMPTY){  
        return false;
    }                   

    *packet_length = a7Data;                                                

    *start_address = pvPortMalloc(a7Data + SIZE_SOF_SOT_DL_CRC_EOP);
    a7_packet_data = *start_address;

    *a7_packet_data = SOF;
    a7_packet_data++;

    *a7_packet_data = SOT;
    a7_packet_data++;

    *a7_packet_data = a7Data; //data length
    a7_packet_data++;                 

    for(int i=0; i<*packet_length; i++){
        if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == pdPASS){   
            *a7_packet_data = a7Data;
            a7_packet_data++;                            
        }
        else{
            vPortFree(*start_address);  
            return false;
        }
    }                        

    if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == errQUEUE_EMPTY){    //CRC
        vPortFree(*start_address); 
        return false;
    }

    *a7_packet_data = a7Data; //crc
    a7_packet_data++; 

    if(xQueueReceive(data_queue_receied_from_a7, &a7Data, pdMS_TO_TICKS(1) == pdTRUE) == errQUEUE_EMPTY){    //EOP
        vPortFree(*start_address); 
        return false;
    }

    *a7_packet_data = a7Data; //eop
    // a7_packet_data++;
    // a7_packet_data -= *packet_length + SIZE_SOF_SOT_DL_CRC_EOP;

    *packet_length += SIZE_SOF_SOT_DL_CRC_EOP;
    
    return true;    
}

void task_send_data_to_skb(void *pvParameters){
    uint8_t skb_received_ack_packet_id;    
    int8_t ackBuf[7] = {SOF, SOT, 3, 0, ACK, 0, EOP};
    int result; //todo: check results    
    int rpmsg_data_length;
    void *a7_rx_buf;
    uint8_t a7_package_length;
    uint8_t *a7_package_address;

    result = rpmsg_rtos_init(0 /*REMOTE_CPU_ID*/, &rdev, RPMSG_MASTER, &app_chnl);
    // PRINTF("Name service handshake is done, M4 has setup a rpmsg channel [%d ---> %d]\r\n", app_chnl->src, app_chnl->dst);
    for (;;){
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        //send ack packets (that were prepared for received skb data packages) to skb
        while(xQueueReceive(ack_queue_of_skb_received_data, &skb_received_ack_packet_id, pdMS_TO_TICKS(1) == pdTRUE)){
            ackBuf[3] = skb_received_ack_packet_id; //packet_id
            ackBuf[5] = 3/*DL*/ + skb_received_ack_packet_id + ACK; //crc
            for(int i=0; i<ACK_PACKAGE_SIZE; i++){ // send ack to skb for all received skb data packages
                debug_putchar(ackBuf[i]);
            }
        }

        if(skb_ack_timeout > TIMEOUT){ //todo: timeout duration is wrong
            PRINTF("timeout\r\n");
            skb_ack_timeout = 0;
            waiting_skb_ack = false;
        }
        else{
            if(waiting_skb_ack){
                //poll ack packages that were received from skb are in the queue
                if(xQueueReceive(ack_queue_received_from_skb, &skb_received_ack_packet_id, pdMS_TO_TICKS(1) == pdTRUE)){
                    if(skb_received_ack_packet_id != sent_skb_packet_packet_id){
                        wrong_ack_received_from_skb = true; //todo: consider flag's usage
                        PRINTF("wrong ack received\r\n");                       
                    }
                    else{
                        PRINTF("ack received\r\n");
                        skb_ack_timeout = 0;
                        waiting_skb_ack = false;
                        sent_skb_packet_packet_id++;
                    }                
                }   
                else{
                    // PRINTF("%d seconds left\r\n", TIMEOUT - skb_ack_timeout);
                    skb_ack_timeout++;
                }             
            }
            else{
                if(switch_event == true){
                    //create switch pressed packet
                    debug_putchar(SOF);//sof
                    debug_putchar(SOT);//sot
                    debug_putchar(DATA_LENGTH_OPCODE_SWITCH_EVENT);//datalength
                    debug_putchar(sent_skb_packet_packet_id);//packet_id
                    debug_putchar(M4_SENSOR_ID);//sid (m4)
                    debug_putchar(OPCODE_SWITCH_EVENT);//opcode
                    debug_putchar(port0_switches);//data0
                    debug_putchar(port1_switches);//data1
                    debug_putchar(DATA_LENGTH_OPCODE_SWITCH_EVENT +
                                  sent_skb_packet_packet_id + 
                                  M4_SENSOR_ID + 
                                  OPCODE_SWITCH_EVENT + 
                                  port0_switches +
                                  port1_switches);//crc
                    debug_putchar(EOP);//eop                    
                    switch_event = false;
                    waiting_skb_ack = true;
                }
                else if(rotary_encoder_event != 0){                    
                    debug_putchar(SOF);//sof
                    debug_putchar(SOT);//sot
                    debug_putchar(DATA_LENGTH_OPCODE_ROTARY_ENCODER_EVENT);//datalength
                    debug_putchar(sent_skb_packet_packet_id);//packet_id
                    debug_putchar(M4_SENSOR_ID);//sid (m4)
                    debug_putchar(OPCODE_ROTARY_ENCODER_EVENT);//opcode
                    debug_putchar(rotary_encoder_event);//data0
                    debug_putchar(DATA_LENGTH_OPCODE_ROTARY_ENCODER_EVENT + 
                                  sent_skb_packet_packet_id +
                                  M4_SENSOR_ID +
                                  OPCODE_ROTARY_ENCODER_EVENT +
                                  rotary_encoder_event);//crc
                    debug_putchar(EOP);//eop             
                    waiting_skb_ack = true;
                    rotary_encoder_event = 0;
                }
                else if(rotary_switch_event == true){
                    debug_putchar(SOF);//sof
                    debug_putchar(SOT);//sot
                    debug_putchar(DATA_LENGTH_OPCODE_ROTARY_SWITCH_EVENT);//datalength
                    debug_putchar(sent_skb_packet_packet_id);//packet_id
                    debug_putchar(M4_SENSOR_ID);//sid (m4)
                    debug_putchar(OPCODE_ROTARY_SWITCH_EVENT);//opcode
                    debug_putchar(rotary_switch_p0);//data0
                    debug_putchar(rotary_switch_p1);//data1
                    debug_putchar(rotary_switch_p2);//data2
                    debug_putchar(DATA_LENGTH_OPCODE_ROTARY_SWITCH_EVENT +
                                  sent_skb_packet_packet_id +
                                  M4_SENSOR_ID +
                                  OPCODE_ROTARY_SWITCH_EVENT + 
                                  rotary_switch_p0 + 
                                  rotary_switch_p1 +
                                  rotary_switch_p2);//crc
                    debug_putchar(EOP);//eop                     
                    waiting_skb_ack = true;
                    rotary_switch_event = false;
                }
                else if(get_packet_from_a7_rx_buffer(&a7_package_length, &a7_package_address)){
                    process_a7_packet(*(a7_package_address + 5), *(a7_package_address + 6));
                    
                    //override packet id, ignore value that came from a7
                    *(a7_package_address + PACKET_ID_OFFSET) = sent_skb_packet_packet_id;
                    
                    for(int i=0; i<a7_package_length; i++){
                        debug_putchar( *(a7_package_address + i));                                          
                    }

                    waiting_skb_ack = true;
                    vPortFree(a7_package_address); 
                }
            }
        }    
        
        //fill data queue with the data that were received from a7
        result = rpmsg_rtos_recv_nocopy(app_chnl->rp_ept, &a7_rx_buf, &rpmsg_data_length, MESSAGE_ADRESS, 1);
        if(result == RPMSG_SUCCESS){                        
            for(int i=0; i<rpmsg_data_length; i++){
                xQueueSendToBack(data_queue_receied_from_a7, (uint8_t *)(a7_rx_buf+i), 0);
            }
        }
        result = rpmsg_rtos_recv_nocopy_free(app_chnl->rp_ept, a7_rx_buf);
        xSemaphoreGive(xSemaphore);
        vTaskDelay(1);
    }
}

void task_fill_skb_rx_buf(void *pvParameters){
    for (;;){
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        if (UART_GetStatusFlag((UART_Type*)BOARD_DEBUG_UART_BASEADDR, uartStatusRxDataReady)){
            if(skb_rec_buf_data_count < MAX_BUF_SIZE){
                skb_rec_buf[skb_rec_buf_wptr] = UART_Getchar((UART_Type*)BOARD_DEBUG_UART_BASEADDR);    
                pointNextPlace(&skb_rec_buf_wptr, 1);
                skb_rec_buf_data_count++;
            }
            else{
                is_rec_buf_full = true;
                PRINTF("skb uart receive buffer is full\r\n");
            }
        }
        xSemaphoreGive(xSemaphore);
        vTaskDelay(1);               
    }
}

void task_parse_skb_rx_buf(void *pvParameters){
    int result;
    void *tx_buf;
    unsigned long size;
    uint16_t len;
    char rx_data;
    uint8_t data_length = 0, packet_id, sensor_id, opcode, checksum, value;
    uint8_t *data_ptr;

    for(;;){
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        while(skb_rec_buf_data_count >= MIN_PKG_SIZE){
            if(skb_rec_buf[skb_rec_buf_rptr] == SOF){
                pointNextPlace(&skb_rec_buf_rptr, 1);
                if(skb_rec_buf[skb_rec_buf_rptr] == SOT){
                    pointNextPlace(&skb_rec_buf_rptr, 1);
                    data_length = skb_rec_buf[skb_rec_buf_rptr];
                    pointNextPlace(&skb_rec_buf_rptr, 1);
                    packet_id = skb_rec_buf[skb_rec_buf_rptr];     
                    pointNextPlace(&skb_rec_buf_rptr, 1);
                    sensor_id = skb_rec_buf[skb_rec_buf_rptr];
                    
                    if(sensor_id == SKB_SENSOR_ID){ 
                        if(skb_rec_buf_data_count >= data_length + SIZE_SOF_SOT_DL_CRC_EOP){
                            pointNextPlace(&skb_rec_buf_rptr, 1);
                            opcode = skb_rec_buf[skb_rec_buf_rptr];
                            data_ptr = pvPortMalloc(data_length - SIZE_PID_SID_OPCOD);
                            value = packet_id + sensor_id + opcode + data_length;
                            pointNextPlace(&skb_rec_buf_rptr, 1);

                            for(int i=0; i<data_length - SIZE_PID_SID_OPCOD; i++){
                                data_ptr[i] = skb_rec_buf[skb_rec_buf_rptr];
                                value += data_ptr[i];
                                pointNextPlace(&skb_rec_buf_rptr, 1);
                            }
                            checksum = skb_rec_buf[skb_rec_buf_rptr];          
                            pointNextPlace(&skb_rec_buf_rptr, 1);

                            if(skb_rec_buf[skb_rec_buf_rptr] == EOP){
                                if(checksum == value){
                                    #if DEBUG_COMM_SKB == 1
                                    PRINTF("data_length:%d\r\n", data_length);
                                    PRINTF("packet_id:%d\r\n", packet_id);
                                    PRINTF("sensor_id:%d\r\n", sensor_id);
                                    PRINTF("opcode:%d\r\ndata:", opcode);
                                    for(int i=0; i<data_length - SIZE_PID_SID_OPCOD; i++){
                                        PRINTF("%d ", data_ptr[i]);
                                    }
                                    PRINTF("\r\nchecksum:%d\r\n", checksum);
                                    #endif

                                    //forward skb packet to a7
                                    tx_buf = rpmsg_rtos_alloc_tx_buffer(app_chnl->rp_ept, &size);
                                    len = 0;
                                    *((char *)(tx_buf + len++)) = SOF;
                                    *((char *)(tx_buf + len++)) = SOT;
                                    *((char *)(tx_buf + len++)) = data_length;
                                    *((char *)(tx_buf + len++)) = packet_id;
                                    *((char *)(tx_buf + len++)) = sensor_id;
                                    for(int i=0; i<data_length-SIZE_PID_SID_OPCOD; i++){
                                        *((char *)(tx_buf + len++)) = data_ptr[i];             
                                    }
                                    *((char *)(tx_buf + len++)) = checksum;
                                    *((char *)(tx_buf + len++)) = EOP;
                                    *((char *)(tx_buf + len++)) = LF;
                                    
                                    //mehmet
                                    result = rpmsg_rtos_send_nocopy(app_chnl->rp_ept, tx_buf, len, MESSAGE_ADRESS);
                                    
                                    //fill ack queue that will be sent to skb
                                    xQueueSendToBack(ack_queue_of_skb_received_data, &packet_id, 0);
                                    process_skb_packet(opcode, data_ptr[0]);     
                                }
                                else{
                                    #if DEBUG_COMM_SKB == 1
                                    PRINTF("DATA CRC ERROR\r\n");
                                    #endif
                                }
                            }                    
                            else{
                                #if DEBUG_COMM_SKB == 1
                                PRINTF("DATA EOP ERROR\r\n");
                                #endif
                            }
                            vPortFree(data_ptr);
                            skb_rec_buf_data_count -= data_length + SIZE_SOF_SOT_DL_CRC_EOP;
                            pointNextPlace(&skb_rec_buf_rptr, 1);
                        }
                        else{
                            // PRINTF("not enough data 1\r\n");
                            pointPreviousPlace(&skb_rec_buf_rptr, SIZE_SOF_SOT_DL_PID);
                            break;
                        }
                    }
                    else if(sensor_id == ACK){
                        value = data_length + packet_id + sensor_id;
                        pointNextPlace(&skb_rec_buf_rptr, 1);
                        checksum = skb_rec_buf[skb_rec_buf_rptr];

                        pointNextPlace(&skb_rec_buf_rptr, 1);
                        if(skb_rec_buf[skb_rec_buf_rptr] == EOP){
                            if(checksum == value){
                                #if DEBUG_COMM_SKB == 1
                                PRINTF("ack data_length:%d\r\n", data_length);
                                PRINTF("ack packet_id:%d\r\n", packet_id);
                                PRINTF("ack sensor_id:%d\r\n", sensor_id);
                                PRINTF("ack checksum:%d\r\n", checksum);
                                #endif
                                //todo: tx buffer update
                                xQueueSendToBack(ack_queue_received_from_skb, &packet_id, 0);
                            }
                            else{
                                #if DEBUG_COMM_SKB == 1
                                PRINTF("ACK CRC ERROR\r\n");
                                #endif
                            }
                        }
                        else{
                            #if DEBUG_COMM_SKB == 1
                            PRINTF("ACK EOP ERROR\r\n");
                            #endif
                        }
                        skb_rec_buf_data_count -= MIN_PKG_SIZE;
                        pointNextPlace(&skb_rec_buf_rptr, 1);                 
                    }                        
                    else{ //UNDEFINED SENSOR ID
                        #if DEBUG_COMM_SKB == 1
                        PRINTF("UNDEFINED PACKET\r\n");
                        #endif
                        skb_rec_buf_data_count -= SIZE_SOF_SOT_DL_CRC_EOP;
                        pointNextPlace(&skb_rec_buf_rptr, 1);
                    }                  
                }
                else{
                    #if DEBUG_COMM_SKB == 1
                    PRINTF("SOT ERROR\r\n");
                    #endif
                    pointNextPlace(&skb_rec_buf_rptr, 1);
                    skb_rec_buf_data_count -= SIZE_SOF_SOT;
                }
            }
            else{
                #if DEBUG_COMM_SKB == 1
                PRINTF("SOF ERROR\r\n");
                #endif
                pointNextPlace(&skb_rec_buf_rptr, 1);
                skb_rec_buf_data_count -= SIZE_SOF;
            }
        }
        xSemaphoreGive(xSemaphore);
        vTaskDelay(5);
    }
}

//todo: update input parameters, use packet start address and packet length
void process_skb_packet(const uint8_t opcode, const uint8_t data){
uint8_t tmp;
    if(opcode == OPCODE_UPDATE_LEDS){
        tmp = data;
        setPin(led1, tmp & 0x01);
        tmp = data;
        setPin(led2, tmp & 0x02);
        tmp = data;
        setPin(led3, tmp & 0x04);
        tmp = data;
        setPin(led4, tmp & 0x08);
        tmp = data;
        setPin(led5, tmp & 0x10);
        tmp = data;
        setPin(led6, tmp & 0x20);    
    }
}

//todo: update input parameters, use packet start address and packet length
void process_a7_packet(const uint8_t opcode, const uint8_t data){
uint8_t tmp;
    if(opcode == OPCODE_UPDATE_LEDS){
        tmp = data;
        setPin(led1, tmp & 0x01);
        tmp = data;
        setPin(led2, tmp & 0x02);
        tmp = data;
        setPin(led3, tmp & 0x04);
        tmp = data;
        setPin(led4, tmp & 0x08);
        tmp = data;
        setPin(led5, tmp & 0x10);
        tmp = data;
        setPin(led6, tmp & 0x20);    
    }
}