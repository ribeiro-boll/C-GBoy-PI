#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "fonte_rgb565_8x8.h"
#define pino_up      16
#define pino_down    12
#define pino_right   26
#define pino_left    13

#define pino_A       4
#define pino_B       17
#define pino_Select  5
#define pino_Start   6
#define pino_Reset   22

#define max_elements 28

typedef struct ROMS{
    int posicao;
    char nome[23];
}ROMS;

bool on_reset_button_phys = false;

bool on_left_button_phys = false;
bool on_right_button_phys = false;
bool on_down_button_phys = false;
bool on_up_button_phys = false;

bool on_A_button_phys = false;
bool on_B_button_phys = false;
bool on_select_button_phys = false;
bool on_start_button_phys = false;
uint16_t frame_buffer[30][30][8][8];
int total_roms = 0;
int index_selecionado = 0;
int index_selecionado_normalizado_em_20_itens = 0;
int start_index_roms = 0;
ROMS* roms = NULL;
char * title = "[ SELECT ROM :D ]";
char* ROMs_path= "ROMs";
int spi;

void handle_reset_button() {
    __atomic_store_n(&on_reset_button_phys,digitalRead(pino_Reset) == LOW,__ATOMIC_RELAXED);
}

void handle_up_button() {
    __atomic_store_n(&on_up_button_phys,digitalRead(pino_up) == LOW,__ATOMIC_RELAXED);
}
void handle_down_button() {
    __atomic_store_n(&on_down_button_phys,digitalRead(pino_down) == LOW,__ATOMIC_RELAXED);
}
void handle_left_button() {
    __atomic_store_n(&on_left_button_phys,digitalRead(pino_left) == LOW,__ATOMIC_RELAXED);
}
void handle_right_button() {
    __atomic_store_n(&on_right_button_phys,digitalRead(pino_right) == LOW,__ATOMIC_RELAXED);
}

void handle_A_button() {
    __atomic_store_n(&on_A_button_phys,digitalRead(pino_A) == LOW,__ATOMIC_RELAXED);
}
void handle_B_button() {
    __atomic_store_n(&on_B_button_phys,digitalRead(pino_B) == LOW,__ATOMIC_RELAXED);
}
void handle_Sel_button() {
    __atomic_store_n(&on_select_button_phys,digitalRead(pino_Select) == LOW,__ATOMIC_RELAXED);
}
void handle_Start_button() {
    __atomic_store_n(&on_start_button_phys,digitalRead(pino_Start) == LOW,__ATOMIC_RELAXED);
}

struct dirent;
void translate_to_framebuffer(uint8_t row, char* phrase, bool is_selected) {
    if (is_selected) {
        for (int i = 0; i<30; i++) {
            for (int j = 0; j<8; j++) {
                for (int k = 0; k<8; k++) {
                    if (strlen(phrase) <i) {
                        frame_buffer[row][i][j][k] = fonte_rgb565_invertida[26][j][k];
                        continue;
                    }
                    if (toupper(phrase[i]) >='A' && toupper(phrase[i]) <= 'Z'& i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_invertida[toupper(phrase[i])-'A'][j][k];
                    else if (toupper(phrase[i]) >='0' && toupper(phrase[i]) <= '9'& i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_numeros_invertida[phrase[i]-'0'][j][k];
                    else if (toupper(phrase[i]) == '.' & i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_invertida[27][j][k];
                    else
                        frame_buffer[row][i][j][k] = fonte_rgb565_invertida[26][j][k];
                }
            }
        }
    }
    else {
        for (int i = 0; i<30; i++) {
            for (int j = 0; j<8; j++) {
                for (int k = 0; k<8; k++) {
                    if (strlen(phrase) <i) {
                        frame_buffer[row][i][j][k] = fonte_rgb565_normal[26][j][k];;
                        continue;
                    }

                    if (toupper(phrase[i]) >='A' && toupper(phrase[i]) <= 'Z' & i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_normal[toupper(phrase[i])-'A'][j][k];
                    else if (toupper(phrase[i]) >='0' && toupper(phrase[i]) <= '9' & i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_numeros_normal[phrase[i]-'0'][j][k];
                    else if (toupper(phrase[i]) == '.' & i<(strlen(phrase)))
                        frame_buffer[row][i][j][k] = fonte_rgb565_normal[27][j][k];
                    else
                        frame_buffer[row][i][j][k] = fonte_rgb565_normal[26][j][k];

                }
            }
        }
    }
}
void search_roms() {
    DIR* diretorio = opendir(ROMs_path);
    if (diretorio == NULL){
        exit(1);
    }
    struct dirent *folder = readdir(diretorio);
    int name_len;
    while (folder!=NULL) {
        name_len = strlen(folder->d_name);
        if (strlen(folder->d_name) <4) {
            folder = readdir(diretorio);
            continue;
        }
        if (folder->d_name[name_len-3] == '.' && folder->d_name[name_len-2] == 'g' && folder->d_name[name_len-1] == 'b' ) {
            if (roms == NULL) {
                total_roms++;
                roms = malloc(sizeof(ROMS) * total_roms);
            }
            else {
                total_roms++;
                roms = realloc(roms,sizeof(ROMS) * total_roms);
            }
            memset(roms[total_roms-1].nome, ' ',22);
            strncpy(roms[total_roms-1].nome,folder->d_name,20);
            //roms[total_roms-1].nome[22] = '\0';
            roms[total_roms-1].posicao = total_roms-1;
        }
        folder = readdir(diretorio);
    }
}

void write_command(uint8_t *command) {
    digitalWrite(25, LOW);
    write(spi, command, 1);
}
void write_data(uint8_t *data, uint16_t bytes) {
    digitalWrite(25, HIGH);
    write(spi, data, bytes);
}
void write_data_16bit(uint16_t *data, uint16_t bytes) {
    digitalWrite(25, HIGH);
    write(spi, data, bytes);
}
void reset_LCD() {
    uint8_t startGram = 0x2c;
    write_command(&startGram);
}
void send_to_lcd(uint16_t *buffer, uint16_t bytes) {
    write_data_16bit(buffer, bytes);
}
void start_ISR() {
    if (wiringPiISR(pino_up, INT_EDGE_BOTH, handle_up_button) < 0) exit(10);
    if (wiringPiISR(pino_down, INT_EDGE_BOTH, handle_down_button) < 0) exit(11);
    if (wiringPiISR(pino_A, INT_EDGE_BOTH, handle_A_button) < 0) exit(14);
}
void start_sys() {
    if (wiringPiSetupGpio()<0)
        exit(1);
    pinMode(18,OUTPUT);
    usleep(15000);
    pinMode(25,OUTPUT);// DC 0 = comando | DC 1 = dados
    usleep(15000);
    pinMode(27, OUTPUT);
    usleep(15000);
    digitalWrite(27, HIGH);
    digitalWrite(18, HIGH);

    pinMode(pino_up, INPUT);
    pinMode(pino_down, INPUT);
    pinMode(pino_left, INPUT);
    pinMode(pino_right, INPUT);

    pinMode(pino_A, INPUT);
    pinMode(pino_B, INPUT);
    pinMode(pino_Select, INPUT);
    pinMode(pino_Start, INPUT);
    pinMode(pino_Reset, INPUT);

    pullUpDnControl(pino_up, PUD_UP);
    pullUpDnControl(pino_down, PUD_UP);
    pullUpDnControl(pino_left, PUD_UP);
    pullUpDnControl(pino_right, PUD_UP);

    pullUpDnControl(pino_A, PUD_UP);
    pullUpDnControl(pino_B, PUD_UP);
    pullUpDnControl(pino_Select, PUD_UP);
    pullUpDnControl(pino_Start, PUD_UP);
    pullUpDnControl(pino_Reset, PUD_UP);
    start_ISR();
}
void pause_subsys() {
    wiringPiISRStop(pino_up);
    wiringPiISRStop(pino_down);
    wiringPiISRStop(pino_A);
}
void shutdown_LCD(int signo) {
    pullUpDnControl(pino_up, PUD_DOWN);
    pullUpDnControl(pino_down, PUD_DOWN);
    pullUpDnControl(pino_left, PUD_DOWN);
    pullUpDnControl(pino_right, PUD_DOWN);

    pullUpDnControl(pino_A, PUD_DOWN);
    pullUpDnControl(pino_B, PUD_DOWN);
    pullUpDnControl(pino_Select, PUD_DOWN);
    pullUpDnControl(pino_Start, PUD_DOWN);
    pullUpDnControl(pino_Reset, PUD_DOWN);
    uint8_t display_off = 0x28;
    uint8_t sleep_in = 0x10;
    pause_subsys();
    write_command(&display_off);
    delay(20);
    write_command(&sleep_in);
    delay(120);
    digitalWrite(18, LOW);
    digitalWrite(27, LOW);
    digitalWrite(25, LOW);
    if (spi >= 0) {
        close(spi);
        spi = -1;
    }
    exit(0);
}
void lcd_start() {
    spi = wiringPiSPISetupMode(0, 32000000, 3);
    uint8_t sequencia[] = {0x1, 0x11, 0x3A, 0x36, 0x21, 0x13, 0x29,0x2A, 0x2B};
    uint8_t dados[] = {0x55,0x0};
    uint8_t contador = 0;
    uint8_t rst = 0x2c;
    for (uint8_t i = 0; i<9;i++) {
        if (i == 2 || i == 3) {
            write_command(&sequencia[i]);
            write_data(&dados[contador], 1);
            contador++;
        }
        else if (i == 7) {
            //bloco pra botar preto
            write_command(&rst);
            uint16_t blk[240] = {0};
            for (int i = 0; i<240; i++){
                send_to_lcd(blk, 240 * 2);
            }
            write_command(&rst);
            uint8_t col[] = {0,0,0,239};
            write_command(&sequencia[i]);
            write_data(col, 4);
        }
        else if (i == 8) {
            uint8_t row[] = {0,0,0,239};
            write_command(&sequencia[i]);
            write_data(row, 4);
        }
        else {
            write_command(&sequencia[i]);
        }
        usleep(15000);
    }
    write_command(&rst);
}

void generate_frame_buffer() {
    //translate_to_framebuffer(0, title, false);
    for (int i = 0; i<max_elements; i++) {
        if (total_roms-1<i){
            //printf("total_roms: %d | i: %d\n",total_roms, i);
            translate_to_framebuffer(i, "                            ", false);
            continue;
        }
        if ((index_selecionado - start_index_roms) == i)
            translate_to_framebuffer(i, roms[i].nome, true);
        else
            translate_to_framebuffer(i, roms[i].nome, false);
    }
    for (int i = max_elements; i< 30 ; i++) {
        translate_to_framebuffer(i, "                            ", false);
    }

}

void send_frame_buffer() {
    reset_LCD();
    uint16_t temp_buffer[240][240];
    uint16_t bytes_faltantes = 240 * 240;
    for (int y = 0; y < 30; y++) {
        for (int x = 0; x < 30; x++) {
            for (int py = 0; py < 8; py++) {
                for (int px = 0; px < 8; px++) {
                    temp_buffer[8 + y * 8 + py][8 + x * 8 + px] =frame_buffer[y][x][py][px];
                }
            }
        }
    }
    for (int i = 0; i<240; i++) {
        send_to_lcd(temp_buffer[i],240 * sizeof(uint16_t));
    }
}

void exec_programa_padrao(){
    pid_t pid = fork();
    if (pid == -1){
        perror("\nerror! failed execution!\n");
        exit(1);
    }
    if (pid == 0) {
        char buff[4096];
        snprintf(buff, 4096, "%s/%s", ROMs_path, roms[index_selecionado].nome);
        printf("%s\n",buff);
        char **argv;
        argv = malloc(sizeof(char*) * 2);
        argv[0] = "C-Gboy";
        argv[1] = strdup(buff);;
        execv("./C-Gboy", argv);
        free(argv[0]);
        free(argv);
        // perror("Erro no execv");
        // _exit(127);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) || WIFEXITED(status) != 0){
        }
        start_ISR();
        lcd_start();
    }
}
void reset_screen() {
    uint8_t rst = 0x2c;
    write_command(&rst);
    uint16_t blk[240] = {0};
    for (int i = 0; i<240; i++){
        send_to_lcd(blk, 240 * 2);
    }
    write_command(&rst);
}

int main() {
    delay(2000);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    signal(SIGTERM,shutdown_LCD);
    signal(SIGHUP,shutdown_LCD);
    signal(SIGINT,shutdown_LCD);
    search_roms();
    //printf("test 2\n");
    start_sys();
    //printf("test 3\n");
    lcd_start();
    //printf("test 4\n");
    while (1) {
        //printf("sel: %d | curr_itens: %d\n",index_selecionado, total_roms);
        if (__atomic_load_n(&on_up_button_phys, __ATOMIC_RELAXED)) {
            //printf("tst 1\n");
            if (index_selecionado - start_index_roms > 0) {
                index_selecionado--;
                if (index_selecionado - 1 < start_index_roms && start_index_roms > 0) {
                    start_index_roms--;
                }
            }
        }
        if (__atomic_load_n(&on_down_button_phys, __ATOMIC_RELAXED)) {
            //printf("tst 2\n");
            if (total_roms - index_selecionado > 1) {
                index_selecionado++;
                if (index_selecionado  > start_index_roms + total_roms + 1 && start_index_roms +  total_roms < max_elements) {
                    start_index_roms++;
                }
            }
        }
        if (__atomic_load_n(&on_A_button_phys, __ATOMIC_RELAXED)) {
            __atomic_store_n(&on_A_button_phys, false, __ATOMIC_RELAXED);
            //printf("tst 3\n");
            close(spi);
            pause_subsys();
            exec_programa_padrao();
        }
        //reset_screen();
        generate_frame_buffer();
        send_frame_buffer();
        delay(100);
    }
}