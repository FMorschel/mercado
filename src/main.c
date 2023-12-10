#include "stm32f4xx.h"
#include "lcd.h"
#include "lcd.c"
#include <string.h>

typedef enum{
	display_catodo,
	display_anodo,
} display_t;

#define DISPLAY display_catodo

#define BAUDGEN_INT 8  //! Divisor baudrate - parte inteira
#define BAUDGEN_FRA 11 //! Divisor baudrate - parte fracionaria
// Baud-rate em 115200

// enumerando segmentos display
typedef enum {
	segmentoA = GPIO_ODR_ODR_0,
	segmentoB = GPIO_ODR_ODR_1,
	segmentoC = GPIO_ODR_ODR_2,
	segmentoD = GPIO_ODR_ODR_3,
	segmentoE = GPIO_ODR_ODR_4,
	segmentoF = GPIO_ODR_ODR_5,
	segmentoG = GPIO_ODR_ODR_6,
} segmento;

const uint32_t segmentosNoDisplay[11] = {
		segmentoA | segmentoB | segmentoC | segmentoD | segmentoE | segmentoF, //0
		segmentoB | segmentoC, //1
		segmentoA | segmentoB | segmentoD | segmentoE | segmentoG, //2
		segmentoA | segmentoB | segmentoC | segmentoD | segmentoG, //3
		segmentoB | segmentoC | segmentoF | segmentoG, //4
		segmentoA | segmentoC | segmentoD | segmentoF | segmentoG, //5
		segmentoA | segmentoC | segmentoD | segmentoE | segmentoF | segmentoG, //6
		segmentoA | segmentoB | segmentoC, //7
		segmentoA | segmentoB | segmentoC | segmentoD | segmentoE | segmentoF | segmentoG, //8
		segmentoA | segmentoB | segmentoC | segmentoD | segmentoF | segmentoG, //9
		segmentoA | segmentoB | segmentoC | segmentoD | segmentoE | segmentoF | segmentoG, //zerar
};

typedef enum {
	num0 = 0,
	num1 = 1,
	num2 = 2,
	num3 = 3,
	num4 = 4,
	num5 = 5,
	num6 = 6,
	num7 = 7,
	num8 = 8,
	num9 = 9,
	numZ = 10,
} numeros_t;

// enumerando displays para a multiplexa��o
typedef enum {
	display0 = GPIO_ODR_ODR_7,
	display1 = GPIO_ODR_ODR_8,
} display;

typedef enum {
	ligada,
	parada,
} estado_contagem;

typedef struct {
	uint8_t contador;
	numeros_t valor_displays[2];
	display ligado_atual;
	estado_contagem estado;
} controle_t;

char bufferLCD[84];
char dadoSerial = 0; // '\0'
controle_t controle = {
		.contador = 0,
		.valor_displays = {num0, num0},
		.ligado_atual = display0,
		.estado = ligada,
	};

void TIM1_UP_TIM10_IRQHandler (void) // arrumar interrup��o
{
	void trocaDisplay(void);
	void ligarSegmentos(void);

	// Limpa o flag de interrupcao
	TIM10->SR &= ~TIM_SR_UIF;

	//passa para o pr�ximo display
	trocaDisplay();

	//ligar segmentos do display ligado_atual
	ligarSegmentos();
}

typedef enum {
	anterior,
	atual,
} estados_infra_opcao;

uint8_t estadosInfra[2] = {0, 0};

int main(void)
{
	void menu();
	// ativando clocks gpios que ser�o ultilizados
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	// ativando clock TIM10
	RCC -> APB2ENR |= RCC_APB2ENR_TIM10EN;

	// sa�das segmentos display
	GPIOB -> MODER &=~ (GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6);
	GPIOB -> MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 |GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0 |GPIO_MODER_MODER4_0 |GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0);

	// sa�das de alimenta��o para os displays
	GPIOB -> MODER &=~ (GPIO_MODER_MODER7 | GPIO_MODER_MODER8);
	GPIOB -> MODER |= (GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0);

	// segmentos do display ir�o piscar em 60hz
	TIM10->PSC = 32;
	TIM10->ARR = 1999;
	TIM10->CR1 = TIM_CR1_CEN;

	// habilitando interrup��o
	TIM10->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0);

	GPIOC -> MODER &=~ GPIO_MODER_MODER8;

	// TODO: ver os pinos para ligar no LCD
	LCD5110_init();
	mensagemPadrao();

	// Configurando funcoes alternativas
	GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3); // Configurando Pino A2 e A3 como funcoeso alternativa
	GPIOA->MODER |= (GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);
	GPIOA->AFR[0] |= 0x07700; // Direcionando funcao alternativa para UART2

	// configurando a porta serial assincrona
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // habilita clock usart 2
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
	// Habilita a usart, habilita transmissao, habilita recepcao
	USART2->CR2 = 0; // Um stop bit
	USART2->CR3 = 0;
	USART2->BRR = ((BAUDGEN_INT << 4) | BAUDGEN_FRA);

	sendSerial("Sistema iniciado com sucesso!\n\r", 31);
	menu();

	while (1)
	{
		estadosInfra[anterior] = estadosInfra[atual];
		if (GPIOC->IDR & GPIO_IDR_IDR_8) {
			estadosInfra[atual] = 1;
			if (estadosInfra[atual] == 0) {
				for(int i = 0; i < 20000; i++);
			}
			if (GPIOC->IDR & GPIO_IDR_IDR_8) {
				estadosInfra[atual] = 1;
			} else {
				estadosInfra[atual] = 0;
			}
		} else {
			estadosInfra[atual] = 0;
		}

		if (estadosInfra[atual] == 1 && estadosInfra[anterior] == 0) {
			setarValorDisplays(controle.contador + 1);
			if (controle.contador == 99) {
				paraEstadoContador();
			} else if (controle.contador > 99) {
				setarValorDisplays(0);
			}
		}

		if (((USART2->SR) & USART_SR_RXNE)) // Recebeu byte ?
		{
			//Limpar flag
			USART2->SR &= ~USART_SR_RXNE;

			dadoSerial = USART2->DR;

			if (dadoSerial == '1') {
				paraEstadoContador();
			} else if (dadoSerial == '2') {
				setarValorDisplays(0);
			} else {
				sendSerial("Comando invalido!\n\r", 19);
				menu();
			}

		} // fim if byte recebido
	}
}

void menu() {
	void sendSerial(char *str, int tamanho);
	sendSerial("Digite 1 para parar a contagem\n\r", 32);
	sendSerial("Digite 2 para zerar a contagem\n\r", 32);
}

void mensagemPadrao(void) {
	LCD5110_clear();
	LCD5110_set_XY(0, 0);
	LCD5110_write_string("Passe os itens pelo");
	LCD5110_set_XY(0, 2);
	LCD5110_write_string("infra-vermelho...");
}

void paraEstadoContador(void) {
	controle.estado = parada;
	// Mostrar no lcd quantos itens passaram
	LCD5110_clear();
	LCD5110_set_XY(0, 0);
	sprintf(bufferLCD, "Itens passados: %d", controle.contador);
	LCD5110_write_string(bufferLCD);
	if (controle.contador == 99) {
		LCD5110_set_XY(0, 2);
		LCD5110_write_string("Limite atingido!");
	}
}

void setarValorDisplays(uint8_t novo_valor) {
	if (controle.estado == parada) return;
	controle.contador = novo_valor;
	if (novo_valor > 99) {
		controle.valor_displays[0] = ~numZ;
		controle.valor_displays[1] = ~numZ;
	} else {
		controle.valor_displays[0] = (numeros_t) (novo_valor / 10);
		controle.valor_displays[1] = (numeros_t) (novo_valor % 10);
	}
	sendSerial("Valor alterado para: ", 21);
	if (novo_valor > 99) {
		sendSerial("--", 2);
	} else {
		sendNumeroSerial(novo_valor);
	}
	sendSerial("\n\r", 2);
}

void sendSerial(char *str, int tamanho) {
	for (int i = 0; i < tamanho; i++) {
		// TODO: ver se precisa esperar
		//while (!(USART2->SR & USART_SR_TXE));
		USART2->DR = str[i];
	}
}

void sendNumeroSerial(int numero) {
	char str[2];
	sprintf(str, "%02d", numero);
	sendSerial(str, 2);
}

void trocaDisplay(void)
{
	switch (controle.ligado_atual)
	{
	case display1:
		controle.ligado_atual = display0;
		break;
	case display0:
	default:
		controle.ligado_atual = display1;
		break;
	}

	if (DISPLAY == display_anodo) {
		GPIOB->ODR &= ~(display0 | display1); // arrumar multiplexa��o display
		GPIOB->ODR |= controle.ligado_atual;
	} else {
		GPIOB->ODR |= (display0 | display1); // arrumar multiplexa��o display
		GPIOB->ODR &= ~controle.ligado_atual;
	}
}

void ligarSegmentos(void)
{
	if (DISPLAY == display_anodo)
	{
		GPIOB->ODR |= segmentosNoDisplay[numZ];
	} else {
		GPIOB->ODR &= ~segmentosNoDisplay[numZ];
	}

	if (DISPLAY == display_catodo){
		switch (controle.ligado_atual)
		{
		case display0:
			GPIOB->ODR |= segmentosNoDisplay[controle.valor_displays[0]];
			break;
		case display1:
			GPIOB->ODR |= segmentosNoDisplay[controle.valor_displays[1]];
			break;
		}
	} else {
		switch (controle.ligado_atual)
		{
		case display0:
			GPIOB->ODR &= ~segmentosNoDisplay[controle.valor_displays[0]];
			break;
		case display1:
			GPIOB->ODR &= ~segmentosNoDisplay[controle.valor_displays[1]];
			break;
		}
	}
}
