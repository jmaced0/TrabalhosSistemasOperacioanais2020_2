#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>


//Macros de parametrizacao
#define NUM_MOLECULAS 100 //quantidade de moleculas de H2O desejadas, o programa será encerrado quando esse valor for atingido
	//Serao criadas 3*NUM_MOLECULAS threads
#define TEMPO 0 //tempo, em milisegundos, programa ficará preso na thread atual antes que seu estado mude, um valor maior ajuda na vizualizacao da execucao do programa em tempo real


//numero de cada elemento para producao de uma molecula de H2O
#define QTD_OXIGENIO 1 
#define QTD_HIDROGENIO 2

int contadorH2O = 0; // contador para checar se producao requisitada ja foi alcancada
int contadorH = 0; //contadores de elementos no buffer
int contadorO = 0;
int pronto = 0;

using namespace std;

// estrutura para representar o buffer, cada buffer tem seu mutex e sua variavel de controle
typedef struct{
	mutex mtx_O;
	mutex mtx_H;
	condition_variable bufferO_livre;
	condition_variable bufferH_livre;
} buffer_t;


//instancia buffers
buffer_t elementos;


void produtorO(){
	while(contadorH2O <= NUM_MOLECULAS) {

		unique_lock<mutex> lock_O(elementos.mtx_O);
		
		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}
		
		
		//adiciona elemento ao buffer
		contadorO++;
		cout << "Oxigenio adicionado ao buffer" << endl;


			
		while ( contadorH < QTD_HIDROGENIO ){ 
			cout << "Esperando elementos para poder produzir H2O" << endl;
			elementos.bufferO_livre.wait(lock_O);
			this_thread::sleep_for(chrono::milliseconds(TEMPO));	
		}

		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}
		
		//é necessario controlar os dois locks para produzir uma molecula, já que são feitas alterações nos dois buffers
		unique_lock<mutex> lock_H(elementos.mtx_H);
		pronto = 1;//variavel para indicar que threads dos hidrogenios podem ser encerradas

		//Remove elementos dos buffers	
		contadorO--;
		contadorH--;
		contadorH--;
			
		//Incrementa contador de moleculas
		cout << "\tH2O produzido. Quantidade atual: " << ++contadorH2O << " moleculas" << endl;
			

		lock_O.unlock();
		lock_H.unlock();
		elementos.bufferO_livre.notify_all();
		elementos.bufferH_livre.notify_all();
	}
}

void produtorH(){
	while(contadorH2O <= NUM_MOLECULAS) {

		unique_lock<mutex> lock_H(elementos.mtx_H);
		

		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}
		
		contadorH++;

		cout << "Hidrogenio adicionado ao buffer" << endl;

		
		while ( pronto == 0 ){ 
			cout << "Esperando elementos para poder produzir H2O" << endl;
			elementos.bufferH_livre.wait(lock_H);
			this_thread::sleep_for(chrono::milliseconds(TEMPO));
		}
		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}
		pronto = 0;
		
		lock_H.unlock();
		elementos.bufferH_livre.notify_all();


	}
}


int main(){

	int idx = 0;

	//instancia as threads
	vector <thread> threads;

	for (idx = 0; idx < NUM_MOLECULAS ; idx++){
		threads.push_back(thread(produtorO));
		threads.push_back(thread(produtorH));
		threads.push_back(thread(produtorH));
	}


	// main() passa a esperar retorno das threads	
	for (idx = 0; idx < 3*NUM_MOLECULAS; idx++){ threads[idx].join();}

	//Espera fim da producao de moleculas
	cout << endl  << contadorH2O << " Moleculas de agua foram produzidas" << endl;
	cout << "Fim do programa" << endl;


	return 0;
}
