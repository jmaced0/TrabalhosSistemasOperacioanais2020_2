#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>


//Macros de parametrizacao
#define MAX_THREADS_OXI 10 //quantidade de threads produzindo Oxigenio
#define MAX_THREADS_HID 10 //quantidade de threads produzindo Hidrogrenio
#define MAX_THREADS_H2O 10 //quantidade maxima de threads produzindo agua
#define MAX_BUFFER_ELEMENTOS 20 //numero maximo de elementos nos vetores de oxigenio e de hidrogenio
#define NUM_MOLECULAS 100 //quantidade de moleculas de H2O desejadas, o programa será encerrado quando esse valor for atingido
#define TEMPO 0 //tempo, em milisegundos, programa ficará preso na thread atual antes que seu estado mude, um valor maior ajuda na vizualizacao da execucao do programa em tempo real


//numero de cada elemento para producao de uma molecula de H2O
#define QTD_OXIGENIO 1 
#define QTD_HIDROGENIO 2

int contadorH2O = 0; // contador para checar se producao requisitada ja foi alcancada

using namespace std;

// estrutura para representar o buffer, cada buffer tem seu mutex e sua variavel de controle
typedef struct{
	vector <string> buffer_elementos;
	mutex mutex_buffer;
	condition_variable buffer_livre; // com 0 oxigenio ou com 0 ou 1 hidrogenio
} buffer_t;



void produtorElemento(buffer_t &buffer, string &elemento){
	while(contadorH2O <= NUM_MOLECULAS) {

		unique_lock<mutex> lock_produtor(buffer.mutex_buffer);

		// se buffer estiver cheio, espere esvaziar
		// alem do tamanho do buffer, é necessário testar a producao de moleculas requisitada já foi atingida,
		// pois uma thread para pode ser reescalonada depois de muito tempo
		while(buffer.buffer_elementos.size() == MAX_BUFFER_ELEMENTOS && contadorH2O < NUM_MOLECULAS){
			buffer.buffer_livre.wait(lock_produtor);
			cout << "Buffer de " << elemento << " cheio" << endl;
			this_thread::sleep_for(chrono::milliseconds(TEMPO));	
		}
		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}

		//adiciona elemento ao buffer
		buffer.buffer_elementos.push_back(elemento);


		//Caso tenha atingido o mínimo necessário de elementos, notifique que os elementos podem consumidos
		if (elemento == "O"){
			if (buffer.buffer_elementos.size() >= QTD_OXIGENIO){
				lock_produtor.unlock();
				buffer.buffer_livre.notify_all();
				cout << "Oxigenio adicionado ao buffer" << endl;
			}

		}
		else{
			if (buffer.buffer_elementos.size() >= QTD_HIDROGENIO){
				buffer.buffer_livre.notify_all(); 
				lock_produtor.unlock();
				cout << "Hidrogenio adicionado ao buffer" << endl;
			}

		}

	}
}


void produtorH2O(buffer_t &bufferOxi, buffer_t &bufferH2){
	while(contadorH2O <= NUM_MOLECULAS) {

		//se não tem elementos suficientes no buffer, espera que eles sejam produzidos
		unique_lock<mutex> lock_H2(bufferH2.mutex_buffer);
		while( bufferH2.buffer_elementos.size() < QTD_HIDROGENIO &&
				contadorH2O < NUM_MOLECULAS){
			cout << "\tEsperando hidrogenio para habilitar producao de H20" << endl;
			bufferH2.buffer_livre.wait(lock_H2);
			this_thread::sleep_for(chrono::milliseconds(TEMPO));	
		}
		//Threads podem ter ficado presas no while, então é necessario testar novamente
		if(contadorH2O >= NUM_MOLECULAS){ break;}

		unique_lock<mutex> lock_Oxi(bufferOxi.mutex_buffer);
		while( bufferOxi.buffer_elementos.size() < QTD_OXIGENIO 
		    		&& contadorH2O < NUM_MOLECULAS ){
			cout << "\tEsperando oxigenio para habilitar producao de H20" << endl;
			bufferOxi.buffer_livre.wait(lock_Oxi);
			this_thread::sleep_for(chrono::milliseconds(TEMPO));	
			cout << "teste " << endl;
		}	
		if(contadorH2O >= NUM_MOLECULAS){ break;}
	
		//Remove elementos dos buffers	
		bufferOxi.buffer_elementos.pop_back();
		bufferH2.buffer_elementos.pop_back();
		bufferH2.buffer_elementos.pop_back();


		//Incrementa contador de moleculas
		cout << "H20 produzido. Quantidade atual: " << ++contadorH2O << " moleculas" << endl;

		//Libera producao de novas moleculas de H2O
		lock_Oxi.unlock();
		lock_H2.unlock();
		bufferH2.buffer_livre.notify_all();
		bufferOxi.buffer_livre.notify_all();
	}

}

int main(){

	int idx = 0;
	string oxi("O");
	string hid("H");

	//instancia buffers
	buffer_t Oxigenio, Hidrogenio;
	//instancia as threads
	vector <thread> threads_oxi, threads_hid, threads_H2O ;
	
	//Chama as threads

	for (idx = 0; idx < MAX_THREADS_OXI; idx++){
		threads_oxi.push_back(thread(produtorElemento,ref(Oxigenio), ref(oxi)));
	}

	for (idx = 0; idx < MAX_THREADS_HID; idx++){
		threads_hid.push_back(thread(produtorElemento,ref(Hidrogenio), ref(hid)));
	}

	for (idx = 0; idx < MAX_THREADS_H2O; idx++){
		threads_H2O.push_back(thread(produtorH2O, ref(Oxigenio), ref(Hidrogenio)));
	}

	// main() passa a esperar retorno das threads	
	for (idx = 0; idx < MAX_THREADS_OXI; idx++){ threads_oxi[idx].join();}
	for (idx = 0; idx < MAX_THREADS_HID; idx++){ threads_hid[idx].join();}
	for (idx = 0; idx < MAX_THREADS_H2O; idx++){ threads_H2O[idx].join();}

	//Espera fim da producao de moleculas
	cout << endl  << contadorH2O << " Moleculas de agua foram produzidas" << endl;
	cout << "Fim do programa" << endl;


	return 0;
}
