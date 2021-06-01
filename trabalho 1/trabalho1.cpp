#include <stdio.h>
#include <unistd.h>
#include </usr/include/sys/types.h>
#include <iostream>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <stdlib.h>

using namespace std;


// Tratador do sinal USR1
void signalHandler1 (int num_sinal){
	cout << "\n\tTarefa 1: executar ping com processo filho\n" << endl;

	pid_t PID_filho = fork(); //tenta criar filho
	int status = 0;
	pid_t aux_pid;

	// Falha ao criar processo filho
	if (PID_filho < 0){
		cout << "Falha ao criar processo filho" << endl;
	}

	// Sucesso em criar processo filho
	// Filho chama exec para executar o ping
	else if (PID_filho == 0) {
		execlp("/bin/ping","ping","8.8.8.8","-c","5",NULL);
	}


	// Pai esperando finalizacao do processo do filho
	else{
		//espera filho morrer
		while((aux_pid = wait(&status)) > 0);
		cout << "\nFilho executou ping com sucesso! \n" << endl;
	}

	// Fontes usadas
	// linuxhint.com/signal_handlers_c_programming_language/
	// pingExample.c disponibilizada pelo professor	
	// https://www.dca.ufrn.br/~adelardo/cursos/DCA409/node36.html
	// codigo que monitor colocou no grupo da disciplina
}


// Tratador do sinal USR2
void signalHandler2 (int num_sinal){
	cout << "\n\tTarefa 2 \n" << endl;
	
	srand(time(nullptr)); // semente baseada no tempo atual
	int pipe1[2], pipe2[2];
	// posicao 0 - Leitura
	// posicao 1 - Escrita

	int num_aleat;
	int num_recebido;
	int retorno;
	int zero = 0;
	int um = 1;

	int parar = 0;
	int idx = 0;
	while (parar == 0){
		idx++;
		
		num_aleat = rand() % 101; //numero aleatorio entre 0 e 100

		// Pipes devem ser criadas antes do fork para manter processos sincronizados
		pipe(pipe1);
		pipe(pipe2);

		//cria processo filho
		pid_t pid_filho = fork();

		if( pid_filho >= 0){ //Sucesso em criar o filho
			//Criacao das pipes
			if (pid_filho == 0){

				//ler dado enviado pelo pai e fecha ponta da pipe
				read(pipe1[0], &num_recebido, sizeof(num_recebido));
				close(pipe1[0]);


				//Verifica se numero recebido eh par e retorna
				// 0 - se par
				// 1 - se impar
				
				if (num_recebido % 2 == 0){
					write(pipe2[1], &zero, sizeof(zero));
				}
				else{ 
					write(pipe2[1], &um, sizeof(um));
				}
				close(pipe2[1]);

				// forca encerramento do filho para seguir com pai
				exit(EXIT_SUCCESS);
			}
			else{
				// envia numero aleatorio para filho e fecha o lado da pipe
				write(pipe1[1], &num_aleat , sizeof(num_aleat));
				close(pipe1[1]);
					
				//Pai espera finalizacao so filho
				int status = 0;
				pid_t aux_pid;
				while((aux_pid = wait(&status)) > 0);
					
				//ler 0 ou 1 enviado pelo filho simbolizando par ou impar, respectivamente
				read(pipe2[0], &retorno, sizeof(retorno));
 				close(pipe2[0]);

				cout << "Iteracao: " << idx;
			        cout <<	"\t numero sorteado: " << num_aleat;
				cout << "\t resposta do filho: " << retorno << endl;
				
				// Encerra tarefa se numero for par
				if (retorno == 0){parar = 1;}
			}
			
		}
	

		else{
			cout << "Falha em criar filho" << endl;
			parar = 1;
		
		}
	}


	// Fontes usadas:
	// https://www.geeksforgeeks.org/pipe-system-call/
	// https://gist.github.com/ecarrara/5174082
	// https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write
	// https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
}

// Tratador do sinal TERM
void signalHandlerTerm (int num_sinal){
	cout << "\n\tPrograma encerrado" << endl;
	exit(0);
}


// Disparador
int main(void){

	pid_t ID_processo = getpid();

	//Imprimir pid do processo
	cout << "PID do processo: " << ID_processo <<  endl;
		// Descricao da funcao getpid no manual
		// getpid()  returns  the process ID (PID) of the calling process.

	// Disparador para os sinais enviados pelo shell
	signal(SIGUSR1, signalHandler1);
	signal(SIGUSR2, signalHandler2);
	signal(SIGTERM, signalHandlerTerm);


	while(1){
		cout << "Esperando sinais ..." << endl;
		sleep(10);
	};

	return 0;
}
