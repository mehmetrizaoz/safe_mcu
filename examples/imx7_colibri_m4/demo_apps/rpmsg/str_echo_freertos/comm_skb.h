#define ESC 0x1b
// #define CR 0x0d
#define LF 0x0a
#define SOF 0x55
#define SOT 0x02
#define EOP 0x10
#define ACK 0x06

#define M4_SENSOR_ID 0x25

#define OPCODE_SWITCH_EVENT 0x41
#define OPCODE_ROTARY_ENCODER_EVENT 0x42
#define OPCODE_ROTARY_SWITCH_EVENT 0x43

#define DATA_LENGTH_OPCODE_SWITCH_EVENT 0x05
#define DATA_LENGTH_OPCODE_ROTARY_ENCODER_EVENT 0x04
#define DATA_LENGTH_OPCODE_ROTARY_SWITCH_EVENT 0x06


#define MAX_BUF_SIZE 256
#define MIN_PKG_SIZE 7
#define SKB_SENSOR_ID 0x22

#define DEBUG_COMM_SKB 0
#define MESSAGE_ADRESS 1024
#define ACK_PACKAGE_SIZE 7

#define SIZE_SOF_SOT_DL_PID 4
#define SIZE_SOF_SOT 2
#define SIZE_SOF 1
#define SIZE_PID_SID_OPCOD 3
#define SIZE_SOF_SOT_DL_CRC_EOP 5
#define PACKET_ID_OFFSET 3

#define TIMEOUT 10000

#define OPCODE_UPDATE_LEDS 0x10

extern void task_fill_skb_rx_buf(void *pvParameters);
extern void task_parse_skb_rx_buf(void *pvParameters);
extern void skb_comm_semaphore_init();
extern void task_send_data_to_skb(void *pvParameters);

extern uint8_t skb_rec_buf[MAX_BUF_SIZE];
extern uint8_t skb_rec_buf_data_count;
extern uint8_t skb_rec_buf_rptr;
extern uint8_t skb_rec_buf_wptr;
