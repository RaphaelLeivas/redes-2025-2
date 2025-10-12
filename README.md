# redes-2025-2
Repositório para códigos da disciplina DCC218 - Redes de Computadores, Engenharia de Sistemas UFMG 2025/2.

# Instalação NS-3

- Diretório de Instalçao no WSL: ~/ns-allinone-3.45 (home/ns-allinone-3.45$)
- Na Jetson Nano: mesmo local
    - vou tentar sem estar com as versoes atualizadas mesmo, pq aqui é ARM


wget https://www.nsnam.org/releases/ns-allinone-3.45.tar.bz2
tar xfj ns-allinone-3.45.tar.bz2
cd ns-allinone-3.45/
cd ns-3.45/
./ns3 configure --enable-examples --enable-tests
./ns3 build



 wget https://www.nsnam.org/releases/ns-3.45.tar.bz2 
 tar xfj ns-3.45.tar.bz2 
 cd ns-3.45/
 ./ns3 build
 

 # Primeiros Passos

 - agora é aprender a desenvolver sobre o ns-3, idealmente com o VS Code e compilando dentro da pasta do ns-3 na home do WSL
    * ou abre o VS Code através do WSL mesmo, ve se isso funcionaria
    * com git e tudo

- roda o codigo do first => só seguir o tutorial deles lá

- adapta igual o GPT para ele gerar o XML de animação com o animation-module

- agora precisamos no NetAnim. ele nao vei o junto pq eu nao instalei o allinone para nao gastar memoria absurda https://www.nsnam.org/docs/models/html/animation.html 

vai para a home (onde esta o ns-3 mesmo)
git clone https://gitlab.com/nsnam/netanim.git

Qt5 (5.4 and over) and CMake are required to build NetAnim

    sudo apt update
    sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools 
    qmake --version

$ cd netanim
$ mkdir build && cd build
$ cmake .. && cmake --build .

temos um executavel em ~/netanim/build/bin$
executa ele: ./netanim

abre a interface, pega o XML e joga nele
    XML gerado ao colocar um arquivo em scratch/myfile, chamar o build, chamar o run,
    ver onde está o XML

Com isso, temos o suficiente para começar o trabalho



# Execução

~/ns-3.45$ ./ns3 build
~/ns-3.45$ ./ns3 run scratch/lab1_p1.cc
            ./ns3 run "scratch/lab1_p1.cc --nPackets=2 --nClients=3"
~/netanim/build/bin$ ./netanim

cd ~/ns-3.45
cd ~/netanim/build/bin



./ns3 run "scratch/lab1_p2.cc --nCsma=2 --verbose=1"
./ns3 run "scratch/lab1_p2.cc --nCsma=2 --nPackets=2"
./ns3 run "scratch/lab1_p2_alt.cc --nCsma=2 --nPackets=2"
./ns3 run "scratch/lab1_p2_alt.cc"

# prox passos

- deixar a parte 1 igual ao first.cc, remover codigos importados
- add a parte 1 no relatorio
- fazer as partes 2 e 3

# PROBLEMAS

- sem VS Code bom => add a extensao do C/C++ la
- sem GDB => ./ns3 run "scratch/lab1_p1.cc --gdb --nPackets=2 --nClients=3"
- sem scripts para automatizar tudo => com dois terminais e os comandos acima funciona ok
- sem git => manualmente copia a pasta scratch para ca e vai comitando