#include "ms4525do.h"
#include <SPI.h>
#include <SD.h>

unsigned long ultimaGravacao = 0;
int pinoCS = 5;
bool sdDisponivel = false;
bool stop = false;

//Limites de pressão de -1.0f a 1.0f PSI (Diferencial)
bfs::Ms4525do pres(&Wire, 0x28, -1.0f, 1.0f);

//Densidade do ar estimada ao nível do mar em kg/m³
const float DENSIDADE_AR = 1.225f; 

const char* NOME_ARQUIVO = "/data_vel.csv";

void iniciarSD(){
  if(!SD.begin(pinoCS)){
    Serial.println("ERRO: Falha ao iniciar o cartao SD");
    sdDisponivel = false;
    return;
  }
  sdDisponivel = true;
  Serial.println("Cartao SD OK");

  if(!SD.exists(NOME_ARQUIVO)){
    File arquivo = SD.open(NOME_ARQUIVO, FILE_WRITE);
    if(arquivo){
      arquivo.println("tempo_s,velocidade_ms");
      arquivo.close();
    }
  }
}

void gravarSD(float tempo_s, float velocidade_ms) {
  if (!sdDisponivel) return;

  File arquivo = SD.open(NOME_ARQUIVO, FILE_WRITE);
  if(arquivo){
    arquivo.print(tempo_s, 2);
    arquivo.print(",");
    arquivo.println(velocidade_ms, 2);
    arquivo.close();
  }else{ 
    Serial.println("Erro ao abrir arquivo para escrita");
  }
}


void lerSD(const char* caminho){
  File arquivo = SD.open(caminho, FILE_READ);
  if(!arquivo){
    Serial.println("Erro ao abrir arquivo para leitura");
    return;
  }
  Serial.println("--- Conteudo do arquivo ---");
  while(arquivo.available()){
    Serial.write(arquivo.read());
  }
  arquivo.close();
  Serial.println("--- Final ---");
}

void setup(){
  Serial.begin(9600);
  while (!Serial) {}
  
  Wire.begin();
  Wire.setClock(100000); 
  
  Serial.println("--- Iniciando SD ---");
  iniciarSD();

  if(!pres.Begin()){
    Serial.println("Erro ao inicializar o sensor MS4525DO!");
    while (1);
  }
}

void loop(){
  if(Serial.available()){
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if(cmd == "ler"){
      stop = true;
      if(!sdDisponivel){
        iniciarSD();
      }
      lerSD(NOME_ARQUIVO);
    } else if(cmd == "apagar"){
      SD.remove(NOME_ARQUIVO);
      Serial.println("Arquivo apagado");
      iniciarSD();
    } else if(cmd == "gravar"){
      stop = false;
    }
  }

  if(!stop){
    if(pres.Read()){
      float pressao_pa = pres.pres_pa(); 

      //Normalizaçã
      if(pressao_pa < 0){ 
        pressao_pa = 0;
      }
      
      //V = sqrt((2 * dp) / rho)
      float velocidade_ms = sqrt((2.0f * pressao_pa) / DENSIDADE_AR);

      //Exibição dos dados
      Serial.print("Pressao: ");
      Serial.print(pressao_pa, 2);
      Serial.print(" Pa | Velocidade: ");
      Serial.print(velocidade_ms, 2);
      Serial.println(" m/s");
      
      if(millis() - ultimaGravacao >= 500){
        gravarSD(millis()/1000.0f, velocidade_ms);
        ultimaGravacao = millis();
      }
    }else{
      Serial.println("Falha ao ler dados do sensor.");
    }
  }

  delay(50); 
}
