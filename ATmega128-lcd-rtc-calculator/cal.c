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

void save_equation(uint8_t key_val)	//배열에 식을 저장
{
	/* Reserve space for '=' and the terminating NUL. */
	if (index >= (int)sizeof(expr) - 2 && key_val != '=' && key_val != '=' && key_val != 'c' && key_val != ' ')
		return;
	expr[index++] = key_val;	//입력한 키에 매핑된 값이 저장됨
	
	lcd_write_data(key_val);
	
	if(key_val == ' ')	//공백 키버튼 누르면 back space
	{
		if(index >=2)	//버퍼에 값이 존재할 때
		{
			index -= 2;	//입력한 공백과 직전 글자 2개 무료화
			expr[index] = '\0'; //버퍼 끝 널 삽입
			
			lcd_write_command(0x10);	//커서 앞으로 두 칸
			_delay_us(50);
			lcd_write_command(0x10);
			_delay_us(50);
			
			lcd_write_data(' ');		//공백으로 덮어서 잔상 없애기
			_delay_us(50);
			
			lcd_write_command(0x10);	//다시 커서 앞으로 한 칸
			_delay_us(50);
		}
		else
		{	//버퍼가 비어있을 때
			index = 0;
			expr[0] = '\0';
			lcd_write_command(0x10);	//입력한 공백 전으로 커서 이동
			_delay_us(50);
		}
		return;
	}
	
	if(key_val == 'c')	//2번 버튼 누르면 화면 클리어
	{
		index = 0;
		expr[0] = '\0';
		printf("clear");
		lcd_clear();
		return;
	}
	
	if(key_val == '=')
	{
		expr[index] = '\0';		//마지막 인덱스에 null삽입
		
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

char **trans(stack* s, char* expr)  //식을 후위표기법으로 변환
{  
	char** res_arr = (char**)calloc(200, sizeof(char*));
	if (res_arr == NULL) return NULL;
	int res_idx = 0;
	int i = 0;
	int len = strlen(expr);
	
	while(i < len){
		
		if(expr[i] == '=') break;
		
		//연속된 숫자를 하나의 문자열로 묶기
		if(isdigit(expr[i]))		//isdigit: 숫자가 아니면 0 반환.
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
		
		char* op = (char*)malloc(sizeof(char) * 2);	//읽은 연산자 문자를 독립된 문자열로 새로 포장
		if (op == NULL) goto allocation_failed;
		op[0] = expr[i];
		op[1] = '\0';
		
		//읽은 문자가 연산자일 때.
		if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')
		{ 
			//스택이 비어있지 않고 스택 안 연산자의 우선운위가 같거나 높은 경우
			while (s->top >= 0 && getPriority(s->data[s->top][0]) >= getPriority(expr[i])) 
			{
				res_arr[res_idx++] = pop(s);	//스택 안 연산자를 pop	
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
			char* open_paren = pop(s); // 스택에 있던 '(' 꺼내서
			free(open_paren);          // 메모리 해제
			free(op);                  // 사용 안 한 현재 ')'의 op도 해제
		}
		else
		{
			free(op);
		}
		i++;
	}
	//문자열을 모두 읽고 스택에 남은 연산자 pop
	while (is_empty(s) != 1) 
	{
		res_arr[res_idx++] = pop(s);
	}
	
	res_arr[res_idx] = NULL;	//끝자리에 null 지정
	return res_arr;

allocation_failed:
	while (!is_empty(s)) free(pop(s));
	for (int j = 0; j < res_idx; j++) free(res_arr[j]);
	free(res_arr);
	return NULL;
}

char *calculate(stack* s, char** postfix) 
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

int getPriority(char op) //연산자 우선순위
{
	if (op == '*' || op == '/') return 2;
	if (op == '+' || op == '-') return 1;
	if (op == '(' || op == ')') return 0;
	return -1;
}

int parsing(char *buff)
{
	int count = 0;
	for (int i = 0; buff[i] != '\0'; i++)
	{
		if (buff[i] == '(') count++;
		else if (buff[i] == ')' && --count < 0) return 1;
	}
	return count != 0;
}

#if 0

void save_equation(uint8_t key_val);
void cal_res(void);

static char eq[32];
static int index = 0;

void save_equation(uint8_t key_val)	//배열에 식을 저장
{
		eq[index++] = key_val;
		if(key_val == '=')	cal_res(); 
		else printf("%c", key_val);
}

void cal_res(void)	// '='이 입력되면 호출해서 결과 계산, 출력
{
	char op = 0;
	char *pop = NULL;
	int res = 0;
	
	if(pop = strchr(eq, '+')) {
		op = '+';
	}
	else if(pop = strchr(eq, '-')) {
		op = '-';
	}
	else if(pop = strchr(eq, '*')) {
		op = '*';
	}
	else if(pop = strchr(eq, '/')) {
		op = '/';
	}
	
	if(*pop == NULL){
		printf("잘못된 수식입니다.\n");
		index = 0;
		eq[0] = '\0';
		return;
	}
	
	*pop = '\0';
	
	int num1 = atoi(eq);
	int num2 = atoi(pop + 1);
	
	switch(op)
	{
		case '+': res = num1 + num2;
		break;
		case '-': res = num1 - num2;
		break;
		case '*': res = num1 * num2;
		break;
		case '/': res = num1 / num2;
		break;	
	}
	
	printf("=%d \n", res);
	
	index = 0;
	eq[0] = 0;	
}
#endif
