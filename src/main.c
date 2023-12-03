#include "stm32f4xx.h"
#include "nokia5110_LCD.h"
// TODO: incluir biblioteca para o nokia

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

//! Manipulador interupcao externa pinos 10 a 15
//! Utilizado rotulo original do arquivo startup_stm32f44x.s
void EXTI15_10_IRQHandler(void)
{
	// Limpa o flag de interrupcao
	EXTI->PR |= EXTI_PR_PR13; // Marca atendimento da interrupção

	if (controle.estado == ligada) {
		if (controle.contador < 99) {
			controle.estado = parada;
			// TODO: mostrar no nokia uma mensagem
		} else {
			setarValorDisplays(0);
		}
	} else {
		controle.estado = ligada;
		setarValorDisplays(0);
		// TODO: limpar mensagem no nokia
	}
}

typedef enum {
	anterior,
	atual,
} estados_infra_opcao;

uint8_t estados_infra[2] = {0, 0};

int main(void)
{
	// ativando clocks gpios que ser�o ultilizados
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

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

	//TODO: definir pino infravermelho
	GPIOC -> MODER &=~ GPIO_MODER_MODER9;

	// PC13 como entrada
	GPIOC -> MODER &=~ GPIO_MODER_MODER13;

	// Selecionando PORT para a interrupção especifica
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;		  // configurando clock para SysCFG
	SYSCFG->EXTICR[3] = SYSCFG_EXTICR4_EXTI13_PC; // Seleciona EXTI13 para port C

	// Habilitando interrupção externa
	EXTI->IMR |= EXTI_IMR_MR13; // Habilita interrupção externa
	EXTI->RTSR |= EXTI_RTSR_TR13; // Habilita interrupção na borda de subida
	EXTI->PR |= EXTI_PR_PR13; // Limpa flag de interrupção

	// Habilitando interrupção no NVIC
	// Rotulo original (EXTI15_10_IRQn) do arquivo stm32f466xx.h
	NVIC_SetPriority(EXTI15_10_IRQn, 1); // Ajusta nivel de prioridade
	NVIC_EnableIRQ(EXTI15_10_IRQn);		 // Habilita interrupcao - rotulo no

	//TODO: configuracao inicial nokia

	while (1)
	{
		estados_infra[anterior] = estados_infra[atual];
		if (GPIOC->IDR & GPIO_IDR_IDR_9) {
			estados_infra[atual] = 1;
			if (estados_infra[atual] == 0) {
				for(int i = 0; i < 20000; i++);
			}
			if (GPIOC->IDR & GPIO_IDR_IDR_9) {
				estados_infra[atual] = 1;
			} else {
				estados_infra[atual] = 0;
			}
		} else {
			estados_infra[atual] = 0;
		}

		if (estados_infra[atual] == 1 && estados_infra[anterior] == 0) {
			setarValorDisplays(controle.contador + 1);
			if (controle.contador == 99) {
				controle.estado = parada;
				//TODO: mostrar no nokia uma mensagem
			} else if (controle.contador > 99) {
				setarValorDisplays(0);
			}
		}

	}
}

void setarValorDisplays(uint8_t novo_valor) {
	if (controle.estado == parada) return;
	controle.contador = novo_valor;
	if (novo_valor > 99) {
		controle.valor_displays[0] = numZ;
		controle.valor_displays[1] = numZ;
	} else {
		controle.valor_displays[0] = (numeros_t) (novo_valor / 10);
		controle.valor_displays[1] = (numeros_t) (novo_valor % 10);
	}
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

	GPIOB->ODR &= ~(display0 | display1); // arrumar multiplexa��o display
	GPIOB->ODR |= controle.ligado_atual;
}

void ligarSegmentos(void)
{
	GPIOB->ODR &= ~segmentosNoDisplay[numZ];

	switch (controle.ligado_atual)
	{
	case display0:
		GPIOB->ODR|=segmentosNoDisplay[controle.valor_displays[0]];
		break;
	case display1:
		GPIOB->ODR|=segmentosNoDisplay[controle.valor_displays[1]];
		break;
	}
}
