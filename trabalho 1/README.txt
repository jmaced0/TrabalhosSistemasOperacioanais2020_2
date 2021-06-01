Sistema operacional usado: Linux
Distribuição: Ubuntu 20.04.1 LTS

Uso do programa:

1) Executar programa pelo shell sem argumentos
2) O ID do processo (pid) do programa será impresso na tela, use esse pid para escolher uma das tarefas do programa:
	2.1) Sinais devem ser disparados para que o programa execute suas funções por meio do tratamento deles, estes sinais são USR1, USR2 e TERM
	2.2) Em outro shell digite uma das opcoes abaixo
		a) kill -USR1 <pid>  
		b) kill -USR2 <pid>
		c) kill -TERM <pid>
	2.3) Onde
		a) Executar tarefa 1: "ping 8.8.8.8 -c 5"
		b) Executar tarefa 2: sorteio de numero [0,100] até que um numero par seja sorteado
		c) Finalizar programa
 


