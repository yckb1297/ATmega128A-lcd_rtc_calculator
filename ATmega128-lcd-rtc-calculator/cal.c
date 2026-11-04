/*
 * cal.c
 *
 * Created: 2026-06-29 16:08:25
 *  Author: kccistc
 */ 

#include "cal.h"
#include "text_lcd.h"

static char expr[100];
static int index = 0;

void save_equation(uint8_t key_val) // 입력 키를 수식에 반영하고 '=' 입력 시 계산
{
	/* 일반 문자 입력 시 '='와 문자열 종료 문자 공간을 남긴다. */
	if (index >= (int)sizeof(expr) - 2 && key_val != '=' && key_val != '=' && key_val != 'c' && key_val != ' ')
		return;
	expr[index++] = key_val; // 키패드/버튼 입력을 수식 버퍼에 저장
	
	lcd_write_data(key_val);
	
	if(key_val == ' ') // 공백 키는 직전 문자 삭제로 처리
	{
		if(index >=2) // 삭제할 문자가 있을 때
		{
			index -= 2; // 방금 넣은 공백과 직전 문자를 버퍼에서 제거
			expr[index] = '\0'; // 변경된 수식 끝 표시
			
			lcd_write_command(0x10); // 방금 표시한 공백 위치로 이동
			_delay_us(50);
			lcd_write_command(0x10);
			_delay_us(50);
			
			lcd_write_data(' '); // LCD에 남은 직전 문자를 지움
			_delay_us(50);
			
			lcd_write_command(0x10); // 다음 입력을 삭제 위치에 표시
			_delay_us(50);
		}
		else
		{ // 삭제할 문자가 없을 때
			index = 0;
			expr[0] = '\0';
			lcd_write_command(0x10); // 표시한 공백 이전으로 커서 복귀
			_delay_us(50);
		}
		return;
	}
	
	if(key_val == 'c') // 계산기 모드의 BTN2: 수식과 화면 초기화
	{
		index = 0;
		expr[0] = '\0';
		printf("clear");
		lcd_clear();
		return;
	}
	
	if(key_val == '=')
	{
		expr[index] = '\0'; // 수식을 문자열로 종료
		
		if (parsing(expr))
		{
			lcd_clear();
			lcd_write_string("Bracket ERROR !!");
			index = 0;
			expr[0] = '\0';
			return;
		}
		init(&s);
		
		char** postfix = trans(&s, expr);
		char* cal_res = postfix ? calculate(&s, postfix) : "ERR";

		lcd_goto_xy(1, 12);
		_delay_us(50);
		for(int i= 0; cal_res[i] != '\0'; i++){
			lcd_write_data(cal_res[i]);
			_delay_us(50);
		}
		
		if (postfix != NULL)
		{
			for (int i = 0; postfix[i] != NULL; i++)
				free(postfix[i]);
			free(postfix);
		}
		index = 0;
		expr[0] = '\0';
	}
	
}

char **trans(stack* s, char* expr) // 중위 수식을 후위 표기 토큰 배열로 변환
{  
	char** res_arr = (char**)calloc(200, sizeof(char*));
	if (res_arr == NULL) return NULL;
	int res_idx = 0;
	int i = 0;
	int len = strlen(expr);
	
	while(i < len){
		
		if(expr[i] == '=') break;
		
		// 연속된 숫자를 하나의 토큰으로 묶는다.
		if(isdigit(expr[i]))
		{
			char* num = (char*)malloc((size_t)len + 1);
			if (num == NULL) goto allocation_failed;
			int num_idx = 0;
			
			while (i < len && (isdigit(expr[i])))
			{
				num[num_idx++] = expr[i++];
			}
			num[num_idx] = '\0';
			res_arr[res_idx++] = num;
		}
		
		char* op = (char*)malloc(sizeof(char) * 2); // 연산자/괄호용 단일 문자 토큰
		if (op == NULL) goto allocation_failed;
		op[0] = expr[i];
		op[1] = '\0';
		
		// 스택의 우선순위가 같거나 높은 연산자를 먼저 출력한다.
		if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')
		{ 
			while (s->top >= 0 && getPriority(s->data[s->top][0]) >= getPriority(expr[i])) 
			{
				res_arr[res_idx++] = pop(s);
			}
			push(s, op);		//현재 연산자를 push

		}
		else if (expr[i] == '(') 
		{
			push(s, op);
		}
		else if (expr[i] == ')') 
		{
			while (s->top >= 0 && s->data[s->top][0]!= '(') 
			{
				res_arr[res_idx++] = pop(s);
			}
			char* open_paren = pop(s); // '('는 결과 토큰에 넣지 않는다.
			free(open_paren);
			free(op); // ')' 토큰도 사용하지 않는다.
		}
		else
		{
			free(op);
		}
		i++;
	}
	// 남은 연산자를 후위 표기 배열에 추가한다.
	while (is_empty(s) != 1) 
	{
		res_arr[res_idx++] = pop(s);
	}
	
	res_arr[res_idx] = NULL; // 토큰 배열의 종료 표시
	return res_arr;

allocation_failed:
	// 변환 중 할당한 토큰과 배열을 모두 해제한다.
	while (!is_empty(s)) free(pop(s));
	for (int j = 0; j < res_idx; j++) free(res_arr[j]);
	free(res_arr);
	return NULL;
}

char *calculate(stack* s, char** postfix) // 후위 표기 수식을 계산하고 결과 문자열 반환
{
	int num_stackarr[100];
	int num_top = -1;
	
	for(int i = 0; postfix[i] != NULL; i++)
	{
		if(isdigit(postfix[i][0]))
		{
			errno = 0;
			char *end;
			long value = strtol(postfix[i], &end, 10);
			if (errno == ERANGE || *end != '\0' ||
			    value < INT_MIN || value > INT_MAX || num_top >= 99)
				return "ERR";
			num_stackarr[++num_top] = (int)value;
		}
		
		else
		{			
			if (num_top < 1) return "ERR";
			int num2 = num_stackarr[num_top--];
			int num1 = num_stackarr[num_top--];
			long res = 0;
	
			switch(postfix[i][0]) 
			{
				case '+': res = (long)num1 + num2; break;
				case '-': res = (long)num1 - num2; break;
				case '*': res = (long)num1 * num2; break;
				case '/': 
					if(num2 == 0) return "ERR";
					res = (long)num1 / num2;
					break;
				default: return "ERR";
			}
			
			if (res < INT_MIN || res > INT_MAX) return "ERR";
			num_stackarr[++num_top] = (int)res;
		}
	}
	
	if (num_top != 0) return "ERR";
	int fin_num = num_stackarr[num_top];

	static char cal_res[20];
	snprintf(cal_res, sizeof(cal_res), "%d", fin_num);
	
	return cal_res;
}


void init(stack* s)
{
	s->top = -1;
}

int is_full(stack* s)
{
	return (s->top == (MAX_STACK_SIZE)-1);
}   // 스택이 가득차면 1

int is_empty(stack* s)
{
	return (s->top == -1);
}   // 스택이 비어있으면 1

void push(stack* s, char *in_char)
{
	if (is_full(s)) {
		printf("스택이 가득찼습니다.\n");
		return;
	}

	s->top++;
	s->data[(s->top)] = in_char;
}

char* pop(stack* s)
{

	if (is_empty(s)) {
		printf("스택이 비어있습니다.\n");
		return 0;
	}
	char *temp = s->data[s->top];
	s->top--;
	return temp;
}

int getPriority(char op) // 숫자가 클수록 연산 우선순위가 높다.
{
	if (op == '*' || op == '/') return 2;
	if (op == '+' || op == '-') return 1;
	if (op == '(' || op == ')') return 0;
	return -1;
}

int parsing(char *buff)
{
	// 괄호가 닫히는 순서와 개수만 검사한다.
	int count = 0;
	for (int i = 0; buff[i] != '\0'; i++)
	{
		if (buff[i] == '(') count++;
		else if (buff[i] == ')' && --count < 0) return 1;
	}
	return count != 0;
}
