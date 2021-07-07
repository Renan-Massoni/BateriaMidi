/*
esse código não foi otimizado e tão pouco foi feito da melhor maneira
se você tem alguma ideia de como melhorar alguma parte, deixe um comentário no meu canal
https://www.youtube.com/channel/UCiRN19kd8weEcCIWcBOrZow
*/

#include <MIDI.h>

// define dos pinos pra caso eu conecte as coisas errado
#define azul A5
#define verde A4
#define vermelho A3
#define amarelo A2
#define branco A1
#define roxo A0
#define pedal 6

// define das notas
// no caso eu usei o midi learning do plugin e achei q essa foi a melhor configuração
// mas você pode tentar colocar as notas q ja estão no plugin, é só olhar a tabela MIDI
#define azulNote 0
#define verdeNote 3
#define vermelhoNote 6
#define amareloNote 9
#define roxoNote 12
#define brancoNote 15
#define pedalNote 18

// esses defines são pura tentativa e erro, se você adicionar mais coisas no código vc VAI TER q mexer neles

#define Limiar 10 // esse define o o valor minimo pro sensor disparar o código
                  // ajuste ele se as notas estiverem disparando muito cedo, ou se vc tiver que bater muito forte
#define sensi 10 // esse define o quanto o código espera pra achar o pico máximo.
                 // ajuste ele se as notas estiverem demorando pra sair, ou se as notas sairem multiplas vezes
#define resSen 15 //esse define o quanto o código espera pra limpar o temp[pos]
                  // ajuste ele se algumas notas sairem esporadicamente ou se as notas não estiverem saindo

MIDI_CREATE_DEFAULT_INSTANCE(); //coisa da biblioteca

// cada batuque tem seu próprio valor de temp,res e Lmax, assim o código não fica sem fazer nada enquanto tiver esperando.
int temp[6] = {0, 0, 0, 0, 0, 0};
int res [6] = {0, 0, 0, 0, 0, 0};
int Lmax[6] = {0, 0, 0, 0, 0, 0};

bool val = false, chave = false;//gambiarras

// não achei função do log de 2 no arduino, mas uma mudança de bases ja faz o serviço
double log2(double n) {
  return log10(n) / log10(2);
}

void leitura (char sensor, int pos) {
  int val = analogRead(sensor);//lê o valor do sensor
  if (val >= Limiar) {//se não for ruído ele dispara
    if (temp[pos] < sensi) {//se não foi o tempo minimo conta mais tempo
      temp[pos]++;
      if (val > Lmax[pos]) {// e salva o valor atual se ele for maior q o Lmax
        Lmax[pos] = val;
      }
    }
    else {//se o tempo for atingido
      temp[pos] = 0;//reseta temp[pos]
      Lmax[pos] = log2(Lmax[pos] + 1);//faz o tratamento da função
      Lmax[pos] = (Lmax[pos] * 127) / 10;//regra de 3 pro limite do midi
      switch (sensor) {//dependendo do sensor, toca a nota correspondente com o volume adequado
        case azul:
          MIDI.sendNoteOn(azulNote, Lmax[pos], 1);
          break;
        case verde:
          MIDI.sendNoteOn(verdeNote, Lmax[pos], 1);
          break;
        case vermelho:
          MIDI.sendNoteOn(vermelhoNote, Lmax[pos], 1);
          break;
        case amarelo:
          MIDI.sendNoteOn(amareloNote, Lmax[pos], 1);
          break;
        case branco:
          MIDI.sendNoteOn(brancoNote, Lmax[pos], 1);
          break;
        case roxo:
          MIDI.sendNoteOn(roxoNote, Lmax[pos], 1);
          break;
      }
      Lmax[pos] = 0;//reseta o Lmax[pos] pra receber a próxima nota
      res[pos] = 0;//reseta o reset
                   //irônico, não é?
    }
  }
  else {//caso o valor seja menor que o limiar
    if (res[pos] < (resSen)) {//começa a contar um tempo
      res[pos]++;
    }
    else {//se muitos valores menores forem contabilizados reseta o temp[pos]
          // eu fiz isso porque as vezes vários micro picos eram acumulados e disparavam os pratos
          // assim, esses micro picos são eliminados se eu não bater no prato
      temp[pos] = 0;
      res[pos] = 0;
    }
  }
}

//o pedal além de ser pull-up é digital
//e eu queria que ele só disparasse uma unica vez quando eu pisasse nele
//então resolvi criar uma função só pra ele
//eu sei que podia ter feito por interrupção da borda de descida, mas sou preguiçoso
void CheckPedal(void) {
  val = digitalRead(pedal);
  if (!val) {//se pisou no pedal
    if (chave == false) {//se a gambiarra estiver desligada
      MIDI.sendNoteOn(pedalNote, 127, 1);//manda a nota e liga a gambiarra
      chave = true;
    }
  } else {//checa se o pedal não está sendo pressionado
    chave = false;//desliga a gambiarra
  }
}

void setup() {
  MIDI.begin();
  Serial.begin(115200);//não sei porque mas só funcionou depois que eu adicionei isso no setup, então fica ai
}

void loop() {
  //faz a leitura de cada batuque e do pedal
  leitura(azul, 0);
  leitura(verde, 1);
  leitura(vermelho, 2);
  leitura(amarelo, 3);
  leitura(branco, 4);
  leitura(roxo, 5);
  CheckPedal();
}
