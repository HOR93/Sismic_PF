#include <msp430.h>
#include <string.h>
// henrique OR.
#define TRUE    1
#define FALSE   0

// Definição do endereço do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR2

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

void lcd_inic(void);
void lcd_aux(char dado);
int pcf_read(void);
void pcf_write(char dado);
int pcf_teste(char adr);
void led_vd(void);
void led_VD(void);
void led_vm(void);
void led_VM(void);
void i2c_config(void);
void gpio_config(void);
void delay(long limite);
void led_char(char x);
void lcd_str(char *pt);
void lcd_cursor (char x);
void lcd_str_n(char *pt, int n);
void md5_shopee(char *input, char *output);
int analise_senha_usuario(char *text);                  // análise da senha
void senha_forte(char *nova_senha);                       //senha forte
void hash_analise(void);                                // analise do has

volatile int senha_nist = 0;
volatile int p1_1 = 0;
volatile int p2_1 = 0;

void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1(void) {
    if (P1IFG & BIT1) {
        p1_1 = 1;
        P1IFG &= ~BIT1;
    }
}


void __attribute__ ((interrupt(PORT2_VECTOR))) Port_2(void) {
    if (P2IFG & BIT1) {
        p2_1 = 1;
        P2IFG &= ~BIT1;
    }
}






int main(void){
    while (TRUE) {
        WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

        gpio_config();
        i2c_config();

        lcd_inic(); // Inicializar LCD
        pcf_write(8); // Acender Back Light

        char senha_input[] = "strongpassword";  // Str0ng!Passw0rdS seraque, strongpassword
        char hash_output[33];  // até 33 o tamanho

        md5_shopee(senha_input, hash_output);  // md5 adaptado

        lcd_clear();
        lcd_str("  UNB - FT");
        __delay_cycles(4000000);
        lcd_clear();
        lcd_str("Lab de Sistemas");
        lcd_cursor(0x40);
        lcd_str("Microprocessados");
        __delay_cycles(4000000);
        lcd_clear();
        lcd_str("Henrique OR");
        __delay_cycles(4000000);

        lcd_clear();
        lcd_str("  Gerador MD5");
        lcd_cursor(0x40);
        lcd_str("Analise de Hash");
        __delay_cycles(2000000);

        lcd_clear();
        lcd_str("Senha escolhida");
        __delay_cycles(2000000);
        lcd_clear();
        lcd_str(senha_input);
        __delay_cycles(4000000);  // 2 segundos

        lcd_clear();
        lcd_str("Analise com LEDS");
        lcd_cursor(0x40);
        lcd_str("VD:OK||VM:RUIM");
        __delay_cycles(5000000);

        lcd_clear();
        if (analise_senha_usuario(senha_input)) {
            lcd_str("Padrao NIST: ");
            lcd_cursor(0x40);
            lcd_str("Senha Segura.");
            senha_nist = 1;
        } else {
            lcd_str("Padrao NIST: ");
            lcd_cursor(0x40);  // Segunda linha, primeira coluna
            lcd_str("Senha Fraca.");
            senha_nist = 0;
        }
        __delay_cycles(6000000);  // 5 segundos

        // timer
        TA0CCR0 = 524;
        TA0CCTL0 = CCIE;
        TA0CTL = TASSEL_2 + MC_1 + ID_3;
        __enable_interrupt();  // interrupções

        //hash
        lcd_clear();
        lcd_str("Carregando Hash");
        __delay_cycles(5000000);
        lcd_clear();
        lcd_str("Quase la..");
        __delay_cycles(5000000);

        lcd_clear();
        lcd_str(hash_output);
        lcd_cursor(0x40);
        lcd_str(hash_output + 9);
        __delay_cycles(10000000);

        P1DIR &= ~BIT1;
        P1REN |= BIT1;   // resistor
        P1OUT |= BIT1;   // pull-up
        P1IE |= BIT1;    // interrupção
        P1IES |= BIT1;   // borda de descida
        P1IFG &= ~BIT1;

        P2DIR &= ~BIT1;
        P2REN |= BIT1;
        P2OUT |= BIT1;
        P2IE |= BIT1;
        P2IES |= BIT1;
        P2IFG &= ~BIT1;


        int continua = 0;

        // pinos
        lcd_clear();
        lcd_str("S1|Analise Hash");
        lcd_cursor(0x40);
        lcd_str("S2|Senha nova");
        __delay_cycles(10000000);

        if (p1_1) {
            char nova_senha[17];
            senha_forte(nova_senha);
            lcd_clear();
            lcd_str("Senha gerada");
            lcd_cursor(0x40);
            lcd_str("aleatoriamente");
            __delay_cycles(5000000);
            lcd_clear();
            lcd_str("Senha Segura: ");
            lcd_cursor(0x40);
            lcd_str(nova_senha);
            __delay_cycles(10000000);
            lcd_clear();
            lcd_str("........");
            continua = 1;
        } else if (p2_1) {
            lcd_clear();
            hash_analise();
            __delay_cycles(10000000);
            lcd_clear();
            lcd_str("........");
            continua = 1;
        } else {
            lcd_clear();
        }
        // voltando ou encerra o programa
        if (!continua) {
            lcd_clear();
            lcd_str("Encerrando..");
            break;
        }
        p1_1 = 0;
        p2_1 = 0;
    }

    return 0;
}

// MD5 adaptado pro codigo
void md5_shopee(char *input, char *output) {
    unsigned int hash[4] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
    unsigned int i, j;
    unsigned int tamanho_input = strlen(input);
    unsigned char *buffer = (unsigned char*)input;

    for (i = 0; i < tamanho_input; i++) {
        hash[i % 4] ^= buffer[i];
        hash[i % 4] = (hash[i % 4] << (i % 4 + 1)) | (hash[i % 4] >> (32 - (i % 4 + 1)));
        hash[i % 4] += buffer[i];
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            unsigned char c = (hash[i] >> (j * 4)) & 0xF;
            if (c < 10) c += '0';
            else c += 'A' - 10;
            output[i * 8 + j] = c;
        }
    }
    output[32] = '\0';
}

int analise_senha_usuario(char *text) {
        int comprimento = strlen(text);
        int maisc = 0, minusc = 0, digito = 0, especial = 0;
        int i;

        if (comprimento < 8) return 0;  // comprimento mínimo

        for (i = 0; i < comprimento; i++) {  // caracteres
            char c = text[i];
            if (isupper((unsigned char)c)) maisc = 1;
            else if (islower((unsigned char)c)) minusc = 1;
            else if (isdigit((unsigned char)c)) digito = 1;
            else especial = 1;
        }

        if (maisc && minusc && digito && especial) return 1;
        return 0;
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    delay(2000);
}

// Função para enviar um comando ao LCD
void lcd_cmd(char cmd) {
    char upper, lower;
    upper = cmd & 0xF0;
    lower = (cmd << 4) & 0xF0;
    pcf_write(upper | 0x0C);
    pcf_write(upper | 0x08);
    pcf_write(lower | 0x0C);
    pcf_write(lower | 0x08);
    delay(1000);
}

// Inicializar LCD modo 4 bits
void lcd_inic(void) {
    UCB0I2CSA = PCF_ADR;  // Endereço Escravo
    UCB0CTL1 |= UCTR | UCTXSTT;  // Gerar START
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0TXBUF = 0;  // Saída PCF = 0;
    while ((UCB0CTL1 & UCTXSTT) == UCTXSTT);  // Esperar STT=0
    if ((UCB0IFG & UCNACKIFG) == UCNACKIFG)  // NACK?
        while(1);

    // Começar inicialização
    lcd_aux(0);  // RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);  // 3
    delay(10000);
    lcd_aux(3);  // 3
    delay(10000);
    lcd_aux(3);  // 3
    delay(10000);
    lcd_aux(2);  // 2

    // Entrou em modo 4 bits
    lcd_aux(2); lcd_aux(8);  // 0x28
    lcd_aux(0); lcd_aux(8);  // 0x08
    lcd_aux(0); lcd_aux(1);  // 0x01
    lcd_aux(0); lcd_aux(6);  // 0x06
    lcd_aux(0); lcd_aux(0xF);  // 0x0F

    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;  // Gerar STOP
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);  // Esperar STOP
    delay(50);
}

// Auxiliar inicialização do LCD (RS=RW=0)
// *** Só serve para a inicialização ***
void lcd_aux(char dado) {
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;  // PCF7:4 = dado;
    delay(50);
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;  // E=1
    delay(50);
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;  // E=0;
}

// Ler a porta do PCF
int pcf_read(void) {
    int dado;
    UCB0I2CSA = PCF_ADR;  // Endereço Escravo
    UCB0CTL1 &= ~UCTR;  // Mestre RX
    UCB0CTL1 |= UCTXSTT;  // Gerar START
    while ((UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP;  // Gerar STOP + NACK
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);  // Esperar STOP
    while ((UCB0IFG & UCRXIFG) == 0);  // Esperar RX
    dado = UCB0RXBUF;
    return dado;
}

// Escrever dado na porta
void pcf_write(char dado) {
    UCB0I2CSA = PCF_ADR;  // Endereço Escravo
    UCB0CTL1 |= UCTR | UCTXSTT;  // Gerar START
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar TXIFG=1
    UCB0TXBUF = dado;  // Escrever dado
    while ((UCB0CTL1 & UCTXSTT) == UCTXSTT);  // Esperar STT=0
    if ((UCB0IFG & UCNACKIFG) == UCNACKIFG)  // NACK?
        while(1);  // Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP;  // Gerar STOP
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);  // Esperar STOP
}

// Testar endereço I2C
// TRUE se recebeu ACK
int pcf_teste(char adr) {
    UCB0I2CSA = adr;  // Endereço do PCF
    UCB0CTL1 |= UCTR | UCTXSTT;  // Gerar START, Mestre transmissor
    while ((UCB0IFG & UCTXIFG) == 0);  // Esperar pelo START
    UCB0CTL1 |= UCTXSTP;  // Gerar STOP
    while ((UCB0CTL1 & UCTXSTP) == UCTXSTP);  // Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0) return TRUE;
    else return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void) {
    UCB0CTL1 |= UCSWRST;  // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC |  // Síncrono
            UCMODE_3 |  // Modo I2C
            UCMST;  // Mestre
    UCB0BRW = BR_100K;  // 100 kbps
    P3SEL |= BIT1 | BIT0;  // Use dedicated module
    UCB0CTL1 = UCSSEL_2;  // SMCLK e remove ressete
}

void led_vd(void) {P4OUT &= ~BIT7;}  // Apagar verde
void led_VD(void) {P4OUT |= BIT7;}  // Acender verde
void led_vm(void) {P1OUT &= ~BIT0;}  // Apagar vermelho
void led_VM(void) {P1OUT |= BIT0;}  // Acender vermelho

// Configurar leds
void gpio_config(void) {
    P1DIR |= BIT0;  // Led vermelho
    P1OUT &= ~BIT0;  // Vermelho Apagado
    P4DIR |= BIT7;  // Led verde
    P4OUT &= ~BIT7;  // Verde Apagado

    // Configuração dos botões
    P1DIR &= ~BIT1;  // P1.1 como entrada
    P1REN |= BIT1;   // Habilitar resistor
    P1OUT |= BIT1;   // Habilitar pull-up
    P1IE |= BIT1;    // Habilitar interrupção
    P1IES |= BIT1;   // Interrupção na borda de descida
    P1IFG &= ~BIT1;  // Limpar flag de interrupção

    P2DIR &= ~BIT1;  // P2.1 como entrada
    P2REN |= BIT1;   // Habilitar resistor
    P2OUT |= BIT1;   // Habilitar pull-up
    P2IE |= BIT1;    // Habilitar interrupção
    P2IES |= BIT1;   // Interrupção na borda de descida
    P2IFG &= ~BIT1;  // Limpar flag de interrupção
}

void delay(long limite) {
    volatile long cont = 0;
    while (cont++ < limite);
}

void led_char(char X) {
    char msn, lsn;
    msn = X & 0xf0;
    lsn = (X & 0xf) << 4;
    pcf_write(msn | 9);
    pcf_write(msn | 0xD);
    pcf_write(msn | 9);
    pcf_write(lsn | 9);
    pcf_write(lsn | 0xD);
    pcf_write(lsn | 9);
}

void lcd_str(char *pt) {
    char i = 0;
    while (pt[i] != '\0') {
        led_char(pt[i]);
        i++;
    }
}

void lcd_cursor(char x) {
    x = x | 0x80;
    char msn = x & 0xF0;
    char lsn = (x & 0x0F) << 4;

    pcf_write(msn | 0x0C);
    pcf_write(msn | 0x08);
    delay(50);
    pcf_write(lsn | 0x0C);
    pcf_write(lsn | 0x08);
    delay(50);
}

void lcd_str_n(char *pt, int n) {
    int i;
    for (i = 0; i < n && pt[i] != '\0'; i++) {
        led_char(pt[i]);
    }
}


void senha_forte(char *nova_senha) {
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int i;


    srand(time(NULL));

    for (i = 0; i < 16; i++) {
        nova_senha[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    nova_senha[16] = '\0';
}

void hash_analise(void) {
    lcd_clear();
    lcd_str("Tipo: MD5");
    lcd_cursor(0x40);
    lcd_str("Tamanho: 128 B");
    __delay_cycles(5000000);
    lcd_clear();
    lcd_str("Tipo: hex");
    lcd_cursor(0x40);
    lcd_str("Caracteres: 32");
    __delay_cycles(5000000);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (senha_nist) {
        P4OUT ^= BIT7;
        P1OUT &= ~BIT0;
    } else {
        P1OUT ^= BIT0;
        P4OUT &= ~BIT7;
    }
}
