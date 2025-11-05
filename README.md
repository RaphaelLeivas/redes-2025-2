# redes-2025-2
Repositório para códigos da disciplina DCC218 - Redes de Computadores, Engenharia de Sistemas UFMG 2025/2.

# Python

# Setting Up Development Environment (Python)

- install python 3.12.4 https://www.python.org/downloads/release/python-3124/
    - make sure the python.exe is placed in the PATH during the installation wizard

- check that the installation was successful: open CMD and run `python -V`. It must show the version installed.
- clone / pull the repository
- go to folder containing project
- connect FUG and PUMP to computer. Make sure they show up in the Operating System's device manager.
- make sure the `setup.json` file has all the configurations used.

Now we need to do the following:
 * Create Pyhton virtual environment
 * Activate the virtual env
 * Install the packages in this virtual environment
 * Run the Jupyer Notebook inside this virtual environment
 
To do all those things, do the following:
- `python -m venv myEnv`
- cd to myEnv/Scripts (cd myEnv/Scripts/)
- `.\activate`
- cd back to project root folder (cd ../../)
- `pip install -r requirements.txt`
- check that all modules were installed: `pip freeze`
- choose the newly created virtual environment in the VS Code lower right corner
- run the program: `python main.py`
- to exit the virtual environment afterwards: `deactivate`

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
./ns3 run "scratch/lab1_p3_alt.cc --nWifi=2 --nPackets=2"
./ns3 run "scratch/lab1_p2_alt.cc"
./ns3 run "scratch/second.cc"
./ns3 run "scratch/lab1_p3.cc"

./ns3 run "scratch/lab2_p1.cc --dataRate="1Mbps" --delay="20ms" --errorRate=0.00001 --nFlows=3" --gdb
./ns3 run "scratch/lab2_p2.cc --dataRate="1Mbps" --delay="20ms" --errorRate=0.00001 --nFlows=3" --gdb

./ns3 run "scratch/bulksend_test.cc --nFlows=5 --simTime=10"

./ns3 run scratch/myfifth.cc

./ns3 run "scratch/tcpvc_test.cc --tracing=true --prefix_name="tcpvc_test""


# prox passos

- deixar a parte 1 igual ao first.cc, remover codigos importados
- add a parte 1 no relatorio
- fazer as partes 2 e 3

# PROBLEMAS

- sem VS Code bom => add a extensao do C/C++ la
- sem GDB => 
    ./ns3 configure --build-profile=debug
    ./ns3 build
    ./ns3 run "scratch/lab1_p1.cc --gdb --nPackets=2 --nClients=3"
- sem scripts para automatizar tudo => com dois terminais e os comandos acima funciona ok
- sem git => manualmente copia a pasta scratch para ca e vai comitando