#include "ms4525do.h"

//Limites de pressao -1.0f a 1.0f PSI
bfs::Ms4525do pres(&Wire, 0x28, -1.0f, 1.0f);

//Densidade do ar estimada ao nível do mar em kg/m³
const float DENSIDADE_AR = 1.225f; 

void setup(){
  Serial.begin(9600);
  while (!Serial) {}
  
  Wire.begin();
  Wire.setClock(100000); //Velocidade I2C de 400kHz
  
  if(!pres.Begin()){
    Serial.println("Erro ao inicializar o sensor MS4525DO!");
    while (1);
  }
  Serial.println("Sensor MS4525DO pronto");
}

void loop(){
  if(pres.Read()){
    float pressao_pa = pres.pres_pa(); 

    //Normalização
    if(pressao_pa < 0){ 
      pressao_pa = 0;
    }
    
    //Equação de Bernoulli: V = sqrt((2 * dp) / rho)
    float velocidade_ms = sqrt((2.0f * pressao_pa) / DENSIDADE_AR);
    
    // m/s para km/h
    float velocidade_kmh = velocidade_ms * 3.6f;
    
    // Exibição dos dados no Monitor Serial
    Serial.print("Pressao: ");
    Serial.print(pressao_pa, 2);
    Serial.println(" Pa | ");
    
    Serial.print("Velocidade: ");
    Serial.print(velocidade_ms, 2);
    Serial.println(" m/s (");
    
  }else{
    Serial.println("Falha ao ler dados do sensor.");
  }
  delay(200); 
}
