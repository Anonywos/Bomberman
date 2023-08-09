/*
 * Computacao Grafica
 * Codigo Exemplo: Rasterizacao de Segmentos de Reta com GLUT/OpenGL
 * Autor: Prof. Laurindo de Sousa Britto Neto
 */

// Bibliotecas utilizadas pelo OpenGL
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <Windows.h>
#include <MMSystem.h>
#include "glm.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
//#include <vector>
#include <forward_list>
#include <iostream>
#include <map>
#include "glut_text.h"
#include "renderers.h"

using namespace std;

// Variaveis Globais
#define FPS 30
#define ESC 27
#define space 32 //Bomba

//Largura e altura da janela
int width = 512, height = 512;
//
int matrizSize = 15;
int mapMatriz[15][15];
int tick = 0;
int enemyTick = 0;
bool enemyWait = false;
int lastMove;
double recoil = 7.0;// 15/2 arredondado
enum GameStates {
	MAIN_MENU,
	GAME_RUNNING,
	GAME_OVER,
	VICTORY
};
GameStates gameState = MAIN_MENU;
int selectedMenuOption = 0;
// float scale = 1.0;
// float rotx = 0.0, roty = 0.0, rotz = 0.0;

//GLfloat angle;
//Enumearacao com os identificadores das animacoes
// enum animation_ids{paused};
// GLManimation * animation = NULL;   //Ponteiro usado no armazenamento de uma animacao
// map <int, GLManimation *> animations; //Mapeamento dos identificadores com as animacoes
// int keyframe = 0; // numero do modelo 3D (keyframe) da animacao que sera desenhado no momento
// int faces = 0, vertices = 0; //Numero de faces e vertices do objeto 3D
// int animation_id = paused;

bool use_gouraud = true; // Determina o uso de Gouraud ou Flat shading

//
struct bomb{
	double x;
	double y;
	double z;
	int tick;
	double range;
	int right, up, down, left;
};

struct Player{
	double xPlayer;
	double yPlayer;
	double zPlayer;
	double speed;
	int bombLimit;
	forward_list<bomb> bombList;
};

Player playerList[2];

void setPlayer(){
	Player p;
	p.xPlayer = 6.0;
	p.yPlayer = 2.0;
	p.zPlayer = 6.0;
	p.speed = 1.0;
	p.bombLimit = 2;
	playerList[0] = p;
}

void setEnemy(){
	Player p;
	p.xPlayer = -6.0;
	p.yPlayer = 2.0;
	p.zPlayer = -6.0;
	p.speed = 1.0;
	p.bombLimit = 1;
	playerList[1] = p;
}

void setBomb(bomb b, int num){
	if(playerList[num].bombLimit > 0){
		b.x = playerList[num].xPlayer;
		b.y = 1.0;
		b.z = playerList[num].zPlayer;
		b.tick = 0;
		b.range = 3;
		b.right = 0;
		b.left = 0;
		b.up = 0;
		b.down = 0;
		playerList[num].bombLimit--;
		playerList[num].bombList.push_front(b);
	}
	
	
}

//fps
void timerCallback(int){
	glutPostRedisplay();
	glutTimerFunc(1000/FPS,timerCallback,0);
}

/*
 * Declaracoes antecipadas (forward) das funcoes (assinaturas das funcoes)
 */
void init(void);
void reshape(int w, int h);
void display(void);
void menu_popup(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);
void drawPixel(int x, int y);
void displayCallback(void);
void drawAnimation(int id, GLuint mode);
//Renderizar mapa
void drawMap();
//função de explosão
void explosion(bomb& b);
//função de colisão
bool isCollidingWallBrick(double x, double z);
//Função principal da IA
void AILogic();
//Função que verifica se o inimigo e personagem estão na mesma reta (linha ou coluna)
bool checkLines();
//Função que calcula a distância de Manhattan
double distanceManhattan(double x1, double z1, double x2, double z2);
//Função que realiza a fuga do inimigo da própria bomba
void enemyScape(int direction);
//colisão com o jogador
bool playerCollision(double x, double z);
//colisão com o inimigo
bool enemyCollision(double x, double z);


/*
 * Funcao principal
 */
int main(int argc, char** argv){
    glutInit(&argc, argv); // Passagens de parametro C para o glut
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); //Selecao do Modo do Display e do Sistema de cor
    glutInitWindowSize (width, height);  // Tamanho da janela do OpenGL
    glutInitWindowPosition (100, 100); //Posicao inicial da janela do OpenGL
    glutCreateWindow ("Bomberman"); // Da nome para uma janela OpenGL
    init(); // Chama funcao init();
    
    glutReshapeFunc(reshape); //funcao callback para redesenhar a tela
    sndPlaySound(TEXT("./sound/background.wav"), SND_ASYNC | SND_LOOP);
    glutTimerFunc(0, timerCallback, 0);//fps
    glutKeyboardFunc(keyboard); //funcao callback do teclado
    glutMouseFunc(mouse); //funcao callback do mouse
    glutPassiveMotionFunc(mousePassiveMotion); //fucao callback do movimento passivo do mouse
    glutDisplayFunc(display); //funcao callback de desenho
    
    // Define o menu pop-up
    glutCreateMenu(menu_popup);
    glutAddMenuEntry("Sair", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    glutMainLoop(); // executa o loop do OpenGL
    return EXIT_SUCCESS; // retorna 0 para o tipo inteiro da funcao main();
}

/*
 * Inicializa alguns parametros do GLUT
 */
void init(void){
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Habilita o modelo de colorização de Gouraud
	glShadeModel(GL_SMOOTH);
	
	// Habilita a definição da cor do material a partir da cor corrente
	glEnable(GL_COLOR_MATERIAL);
	
	// Habilita a definição da cor do material a partir da cor corrente
	glEnable(GL_COLOR_MATERIAL);
	//Habilita o uso de iluminação
	glEnable(GL_LIGHTING);  
	// Habilita a luz de número 0
	glEnable(GL_LIGHT0);
    
    glEnable(GL_DEPTH_TEST); // Habilita o algoritmo Z-Buffer
    
    // Configura a posição e cor da luz
    // GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 }; // (x, y, z, w)
    // GLfloat light_diffuse[] = { 1.0, 0.0, 0.0, 0.5 };  // (R, G, B, A)
    // 
    // glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    
    // int brickI = 0;
    // Brick brick;
	for(int i = 0; i < matrizSize; i++){
		for(int j = 0; j < matrizSize; j++){
			if(i == 0 || j == 0 || i == matrizSize-1 || j == matrizSize-1){
				mapMatriz[i][j] = 1;
			}else if(i%2 == 0 && j%2 == 0){
				mapMatriz[i][j] = 1;
			}else if(i >= 3 && i <= 11){
				mapMatriz[i][j] = 2;
			}else if(j >= 3 && j <= 11){
				mapMatriz[i][j] = 2;
			}else{
				mapMatriz[i][j] = 0;
			}
			
		}
	}
	
	// for(int i = 0; i < 163; i++){
	// 	brick = bricks[i];
	// }
	
	setPlayer();
	setEnemy();
    /*
     *  Carregando Modelos 3Ds, Texturas e Animacoes
     */
    // animation = glmLoadAnimation("models/ironman-T-pose.obj", paused, 1);
    // animation->name = "paused";
    // animations[paused] = animation;
    // faces = animation->models[0]->numtriangles;
    // vertices = animation->models[0]->numvertices;
    //animation = glmLoadAnimation("models/ironman/animations/idle", idle, 50);
    //animations[idle] = animation;
    
}

/*
 * Ajusta a projecao para o redesenho da janela
 */
void reshape(int w, int h)
{
	// Muda para o modo de projecao e reinicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);

    // Define a forma do volume de visualizacao para termos
    // uma projecao perspectiva (3D).
    gluPerspective(45, (float)w/(float)h , 1.0, 100.0); //(angulo, aspecto, ponto_proximo, ponto distante)
    gluLookAt(0.0,20.0,10.0,  // posicao da camera (olho) defalt: 0.0,0.0,7.0
              0.0,1.0,0.0,  // direcao da camera (geralmente para centro da cena)
              0.0,1.0,0.0); // sentido ou orientacao da camera (de cabeca para cima)

   // muda para o modo de desenho
	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();

}

/*
 * Controla os desenhos na tela
 */
void display(){
	
	if(gameState == MAIN_MENU){
		displayCallback();
	}else if(gameState == GAME_RUNNING){
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Limpa o Buffer de Cores
		glClearColor (0.03, 0.67, 0.9, 0.8); 
	    glLoadIdentity();
	    
	    // GLuint mode;
	    // if(use_gouraud) mode = (GLM_SMOOTH | GLM_TEXTURE);
	    // else mode = (GLM_FLAT  |GLM_TEXTURE);
	    
	    
	    drawMap();
	    
	    // player
	    glPushMatrix();
	    	glTranslatef(playerList[0].xPlayer, playerList[0].yPlayer, playerList[0].zPlayer);
	        glScalef (1.0, 5.0, 1.0);
	        glColor3f(1.0,1.0,0.0);
	        glutSolidCube(1.0); // Tamanho
	        glShadeModel(GL_SMOOTH);
	    glPopMatrix();
	    
	    //Enemy
		glPushMatrix();
	    	glTranslatef(playerList[1].xPlayer, playerList[1].yPlayer, playerList[1].zPlayer);
	        glScalef (1.0, 5.0, 1.0);
	        glColor3f(1.0,0.0,0.5);
	        glutSolidCube(1.0); // Tamanho
	        glShadeModel(GL_SMOOTH);
	    glPopMatrix();  
	    
	    if(enemyWait == true){
			enemyTick++;
			if(enemyTick > 75){
				enemyTick = 0;
				playerList[1].bombLimit = 1;
				enemyWait = false;
			}
		}
		if(tick >= 15){
			AILogic(); 
			tick = 0;
		}
		tick++;
		
	    //Bombs player
	    for(forward_list<bomb>::iterator b = playerList[0].bombList.begin(); b != playerList[0].bombList.end(); b++){
	        if(b->tick < 60){
				glPushMatrix();
			    	glTranslatef(b->x, b->y, b->z);
			        glScalef (1.0, 1.5, 1.0);
			        glColor3f(0.0,0.0,0.0);
			        //glutSolidCube(1.0); // Tamanho
			        glutSolidSphere(0.7, 10, 10); 
			        glShadeModel(GL_SMOOTH);
	    		glPopMatrix();
	    		b->tick += 1; 
			}else{
				if(b->tick < 65){
					explosion(*b);
					b->tick += 1;
				}else{
					 b = playerList[0].bombList.erase_after(playerList[0].bombList.begin());
					 playerList[0].bombLimit++;
				}
			}
			
	    }
	    
	    //Bombs enemy
	    for(forward_list<bomb>::iterator b = playerList[1].bombList.begin(); b != playerList[1].bombList.end(); b++){
			if(b->tick < 60){
				glPushMatrix();
			    	glTranslatef(b->x, b->y, b->z);
			        glScalef (1.0, 1.5, 1.0);
			        glColor3f(0.0,0.0,0.0);
			        //glutSolidCube(1.0); // Tamanho
			        glutSolidSphere(0.7, 10, 10);
			        glShadeModel(GL_SMOOTH);
	    		glPopMatrix();
	    		b->tick += 1; 
			}else{
				if(b->tick < 65){
					explosion(*b);
					b->tick += 1;
				}else{
					 //b->tick = 0;
					 b = playerList[1].bombList.erase_after(playerList[1].bombList.begin());
				}
			}
			
	    }
	    
	    // origem volta para o sistema de coordenadas original
	    //glPopMatrix();
		
	    // Troca os buffers, mostrando o que acabou de ser desenhado
	    glutSwapBuffers();
	}else if(gameState == GAME_OVER){
		displayCallback();
	}else if(gameState == VICTORY){
		displayCallback();
	}
    
}

void displayCallback(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor (0.0, 0.0, 0.0, 1.0); 
    glLoadIdentity();

    if (gameState == MAIN_MENU){
		renderMainMenu(selectedMenuOption, 0.0, 0.0, 0.0, 10.0);
	}else if (gameState == GAME_OVER){
		glClearColor (0.9, 0.67, 0.3, 0.8);
		renderGameOver(selectedMenuOption, 0.0, 0.0, 0.0, 10.0);
	} else if (gameState == VICTORY){
		glClearColor (0.03, 0.67, 0.9, 0.8); 
		renderVictory(selectedMenuOption, 0.0, 0.0, 0.0, 10.0);
	}

    glutSwapBuffers();
}

/*
 * Controla o menu pop-up
 */
void menu_popup(int value){
    
}


/*
 * Controle das teclas comuns do teclado
 */
void keyboard(unsigned char key, int x, int y){
	double currentX;
	double currentZ;
	double currentSpeed;
	
    switch (key) { // key - variavel que possui valor Abool escalaP = true;//escala positivaSCII da tecla precionada
		case 'a': 
			if(gameState == MAIN_MENU){
				gameState = GAME_RUNNING;
			}else if(gameState == GAME_RUNNING){
				currentX = playerList[0].xPlayer; 
				currentZ = playerList[0].zPlayer;
				currentSpeed = playerList[0].speed;
				if(isCollidingWallBrick(currentX-currentSpeed, currentZ) == false){
					playerList[0].xPlayer-=playerList[0].speed;
				}
			}else if(gameState == GAME_OVER){
				exit(EXIT_SUCCESS);
			}else if(gameState == VICTORY){
				exit(EXIT_SUCCESS);
			}
			
		break;
		case 's': 
			if(gameState == MAIN_MENU){
				exit(EXIT_SUCCESS);
			}else if(gameState == GAME_RUNNING){
				currentX = playerList[0].xPlayer; 
				currentZ = playerList[0].zPlayer;
				currentSpeed = playerList[0].speed;
				if(isCollidingWallBrick(currentX, currentZ+currentSpeed) == false){
					playerList[0].zPlayer+=playerList[0].speed;
				}
			}
			
		break; 
		case 'd': 
			currentX = playerList[0].xPlayer; 
			currentZ = playerList[0].zPlayer;
			currentSpeed = playerList[0].speed;
			if(isCollidingWallBrick(currentX+currentSpeed, currentZ) == false){
				playerList[0].xPlayer+=playerList[0].speed;
			}
		break;
		case 'w': 
			currentX = playerList[0].xPlayer; 
			currentZ = playerList[0].zPlayer;
			currentSpeed = playerList[0].speed;
			if(isCollidingWallBrick(currentX, currentZ-currentSpeed) == false){
				playerList[0].zPlayer-=playerList[0].speed;
			}
		break;
		case space: 
			bomb b;
			setBomb(b, 0);
		break;
		case ESC: exit(EXIT_SUCCESS); break;
    }
}

/*
 * Controle dos botoes do mouse
 */
void mouse(int button, int state, int x, int y){
	
 //    switch (button) {
 //        case GLUT_LEFT_BUTTON:
	// }
}
/*
 * Controle da posicao do cursor do mouse
 */
void mousePassiveMotion(int x, int y){
    glutPostRedisplay();
}

// void drawAnimation(int id, GLuint mode){
//     glmDrawAnimation(animations[id], keyframe, mode);
// }

void drawMap(){
	//glTranslatef((double)i-recoil, 0.0, (double)j-recoil);
	//glTranslatef((double)j-recoil, 0.0, (double)i-recoil);
	for(int i = 0; i < matrizSize; i++){
		for(int j = 0; j < matrizSize; j++){
			if(mapMatriz[i][j] == 0){//floor
				glPushMatrix();
			    	glTranslatef((double)j-recoil, 0.0, (double)i-recoil);
			    	glScalef(1.0, 1.0, 1.0);
			    	glColor3f(0.0, 1.0, 0.0); 
			    	glutSolidCube(1.0);
		    	glPopMatrix();
			}else if(mapMatriz[i][j] == 1){//wall
				glPushMatrix();
			    	glTranslatef((double)j-recoil, 1.0, (double)i-recoil);
			    	glScalef(1.0, 2.0, 1.0);
			    	glColor3f(1.0, 1.0, 1.0); //white
			    	glutSolidCube(1.0);
		    	glPopMatrix();
			}else if(mapMatriz[i][j] == 2){//brick
				glPushMatrix();
					glTranslatef((double)j-recoil, 1.0, (double)i-recoil);
			    	glScalef(1.0, 2.0, 1.0);
			    	glColor3f(1.0, 0.0, 0.0); 
			    	glutSolidCube(1.0);
			    glPopMatrix();
			    
				glPushMatrix();
					glTranslatef((double)j-recoil, 0.0, (double)i-recoil);
			    	glScalef(1.0, 1.0, 1.0);
			    	glColor3f(0.0, 1.0, 0.0); 
			    	glutSolidCube(1.0);
			    glPopMatrix();
			}
		}
	}
}

void explosion(bomb& b){
	double x = b.x;
	double y = b.y;
	double z = b.z;
	double range = b.range;
	
	double translucent = 0.5;
	int xCor, zCor;
	
	glPushMatrix();
	    // GLfloat object_emissive_color[] = { 0.1, 0.0, 0.0, 0.3 }; // (R, G, B, A)
    	// glMaterialfv(GL_FRONT, GL_EMISSION, object_emissive_color);
    	glTranslatef(x, y, z);
    	glColor4f(1.0, 0.0, 0.0, translucent); 
    	glutSolidSphere(0.7, 10, 10);
	glPopMatrix();
	if(playerCollision(x, z)){
		gameState = GAME_OVER;
		sndPlaySound(TEXT("./sound/levelLost.wav"), SND_ASYNC);
	}else if(enemyCollision(x, z)){
		gameState = VICTORY;
		sndPlaySound(TEXT("./sound/levelWon.wav"), SND_ASYNC);
	}
	for(int i = 1; i < range; i++){
		//logic right side bomb 
		if(b.right == 0){
			if(playerCollision(x+i, z)){
				gameState = GAME_OVER;
				sndPlaySound(TEXT("./sound/levelLost.wav"), SND_ASYNC);
			}else if(enemyCollision(x+i, z)){
				gameState = VICTORY;
				sndPlaySound(TEXT("./sound/levelWon.wav"), SND_ASYNC);
			}else if(isCollidingWallBrick(x+i, z) == false){
				glPushMatrix();
			    	glTranslatef(x+i, y, z);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}else{
				b.right = i;
				xCor = (int)x+i+recoil;
				zCor = (int)z+recoil;
				if(mapMatriz[zCor][xCor] == 2){
					mapMatriz[zCor][xCor] = 0;
					glPushMatrix();
				    	glTranslatef(x+i, y, z);
				    	glColor4f(1.0, 0.0, 0.0, translucent); 
				    	glutSolidSphere(0.7, 10, 10);
					glPopMatrix();
				}
			}
		}else if(b.right >= i){
			xCor = (int)x+i+recoil;
			zCor = (int)z+recoil;
			if(mapMatriz[zCor][xCor] == 0){
				glPushMatrix();
			    	glTranslatef(x+i, y, z);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}
		}
		//logic left side bomb
		if(b.left == 0){
			if(playerCollision(x-i, z)){
				gameState = GAME_OVER;
				sndPlaySound(TEXT("./sound/levelLost.wav"), SND_ASYNC);
			}else if(enemyCollision(x-i, z)){
				gameState = VICTORY;
				sndPlaySound(TEXT("./sound/levelWon.wav"), SND_ASYNC);
			}else if(isCollidingWallBrick(x-i, z) == false){
				glPushMatrix();
			    	glTranslatef(x-i, y, z);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}else{
				b.left = i;
				xCor = (int)x-i+recoil;
				zCor = (int)z+recoil;
				if(mapMatriz[zCor][xCor] == 2){
					mapMatriz[zCor][xCor] = 0;
					glPushMatrix();
				    	glTranslatef(x-i, y, z);
				    	glColor4f(1.0, 0.0, 0.0, translucent); 
				    	glutSolidSphere(0.7, 10, 10);
					glPopMatrix();
				}
			}
		}else if(b.left >= i){
			xCor = (int)x-i+recoil;
			zCor = (int)z+recoil;
			if(mapMatriz[zCor][xCor] == 0){
				glPushMatrix();
			    	glTranslatef(x-i, y, z);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}
		}
		//logic down side bomb
		if(b.down == 0){
			if(playerCollision(x, z+i)){
				gameState = GAME_OVER;
				sndPlaySound(TEXT("./sound/levelLost.wav"), SND_ASYNC);
			}else if(enemyCollision(x, z+i)){
				gameState = VICTORY;
				sndPlaySound(TEXT("./sound/levelWon.wav"), SND_ASYNC);
			}else if(isCollidingWallBrick(x, z+i) == false){
				glPushMatrix();
			    	glTranslatef(x, y, z+i);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}else{
				b.down = i;
				xCor = (int)x+recoil;
				zCor = (int)z+i+recoil;
				if(mapMatriz[zCor][xCor] == 2){
					mapMatriz[zCor][xCor] = 0;
					glPushMatrix();
				    	glTranslatef(x, y, z+i);
				    	glColor4f(1.0, 0.0, 0.0, translucent); 
				    	glutSolidSphere(0.7, 10, 10);
					glPopMatrix();
				}
			}
		}else if(b.down >= i){
			xCor = (int)x+recoil;
			zCor = (int)z+i+recoil;
			if(mapMatriz[zCor][xCor] == 0){
				glPushMatrix();
			    	glTranslatef(x, y, z+i);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}
		}
		//logic up side bomb
		if(b.up == 0){
			if(playerCollision(x, z-i)){
				gameState = GAME_OVER;
				sndPlaySound(TEXT("./sound/levelLost.wav"), SND_ASYNC);
			}else if(enemyCollision(x, z-i)){
				gameState = VICTORY;
				sndPlaySound(TEXT("./sound/levelWon.wav"), SND_ASYNC);
			}else if(isCollidingWallBrick(x, z-i) == false){
				glPushMatrix();
			    	glTranslatef(x, y, z-i);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}else{
				b.up = i;
				xCor = (int)x+recoil;
				zCor = (int)z-i+recoil;
				if(mapMatriz[zCor][xCor] == 2){
					mapMatriz[zCor][xCor] = 0;
					glPushMatrix();
				    	glTranslatef(x, y, z-i);
				    	glColor4f(1.0, 0.0, 0.0, translucent); 
				    	glutSolidSphere(0.7, 10, 10);
					glPopMatrix();
				}
			}
		}else if(b.up >= i){
			xCor = (int)x+recoil;
			zCor = (int)z-i+recoil;
			if(mapMatriz[zCor][xCor] == 0){
				glPushMatrix();
			    	glTranslatef(x, y, z-i);
			    	glColor4f(1.0, 0.0, 0.0, translucent); 
			    	glutSolidSphere(0.7, 10, 10);
				glPopMatrix();
			}
		}
	}
}

bool isCollidingWallBrick(double x, double z){
	for(int i = 0; i < matrizSize; i++){
		for(int j = 0; j < matrizSize; j++){
			if(mapMatriz[i][j] != 0){
				if(x == (double)j-recoil && x+1.0 == (double)j-recoil+1.0){
					if(z == (double)i-recoil && z+1.0 == (double)i-recoil+1.0){
						return true;
					}
				}
			}
		}
	}
	return false;
}

void AILogic(){
	double enemyX = playerList[1].xPlayer;
	double enemyZ = playerList[1].zPlayer;
	double speedE = playerList[1].speed;//speed Enemy
	double playerX = playerList[0].xPlayer;
	double playerZ = playerList[0].zPlayer;
	double distLeft, distRight, distUp, distDown;
	int x, z;
	double diffPositionX = playerX - enemyX;//diferença das posições x
	double diffPositionZ = playerZ - enemyZ;//diferença das posições x
	int direction;//0 = left, 1 = right, 2 = up, 3 = down
	
	// if(playerList[0].xPlayer == 5 && playerList[0].zPlayer == 6){
	// 	int w = 0;
	// }
	
	if(checkLines() == true && enemyWait == false){
		bomb b;
		setBomb(b, 1);
		enemyScape(lastMove);
		enemyWait = true;
	}else if(enemyWait == false){
		x = enemyX+recoil;
		z = enemyZ+recoil;
		//simplismente fazendo a difereça ser um valor positivo
		if(diffPositionX < 0){
			diffPositionX*=-1;
		}
		if(diffPositionZ < 0){
			diffPositionZ*=-1;
		}
		//------------------------------------------------------
		
		//left
		if(mapMatriz[z][x-1] != 1){
			distLeft = distanceManhattan(enemyX - speedE, enemyZ, playerX, playerZ);
		}else{
			distLeft = 100.0;
		}
		//right
		if(mapMatriz[z][x+1] != 1){
			distRight = distanceManhattan(enemyX + speedE, enemyZ, playerX, playerZ);
		}else{
			distRight = 100.0;
		}
		//down
		if(mapMatriz[z+1][x] != 1){
			distDown = distanceManhattan(enemyX, enemyZ+speedE, playerX, playerZ);
		}else{
			distDown = 100.0;
		}
		//up
		if(mapMatriz[z-1][x] != 1){
			distUp = distanceManhattan(enemyX, enemyZ-speedE, playerX, playerZ);
		}else{
			distUp = 100.0;
		}
		
		//Escolher a direção
		if(diffPositionZ >= diffPositionX){
			bool down = false;
			
			if(distDown <= distUp){
				//min = distDown;
				direction = 3;
				down = true;
			}else{
				//min = distDown;
				direction = 2;
			}
			if(down){
				if(distLeft < distDown || distRight < distDown){
					if(distLeft <= distRight){
						direction = 0;
					}else {
						direction = 1;
					}
				}
			}else{
				if(distLeft < distUp || distRight < distUp){
					if(distLeft <= distRight){
						direction = 0;
					}else {
						direction = 1;
					}
				}
			}
		}else{
			bool left = false;
			
			if(distLeft <= distRight){
				direction = 0;
				left = true;
			}else{
				direction = 1;
			}
			if(left){
				if(distDown < distLeft || distUp < distLeft){
					if(distDown <= distUp){
						direction = 3;
					}else {
						direction = 2;
					}
				}
			}else{
				if(distDown < distRight || distUp < distRight){
					if(distDown <= distUp){
						direction = 3;
					}else {
						direction = 2;
					}
				}
			}
		}
		
		//drop bomb
		if(direction == 0){//left
			if(mapMatriz[z][x-1] == 0){
				playerList[1].xPlayer-=speedE;
				lastMove = direction;
			}else if(mapMatriz[z][x-1] == 2){
				bomb b;
				setBomb(b, 1);
				enemyScape(lastMove);
				enemyWait = true;
			}
		}else if(direction == 1){//right
			if(mapMatriz[z][x+1] == 0){
				playerList[1].xPlayer+=speedE;
				lastMove = direction;
			}else if(mapMatriz[z][x+1] == 2){
				bomb b;
				setBomb(b, 1);
				enemyScape(lastMove);
				enemyWait = true;
			}
		}else if(direction == 2){//up
			if(mapMatriz[z-1][x] == 0){
				playerList[1].zPlayer-=speedE;
				lastMove = direction;
			}else if(mapMatriz[z-1][x] == 2){
				bomb b;
				setBomb(b, 1);
				enemyScape(lastMove);
				enemyWait = true;
			}
		}else if(direction == 3){//down
			if(mapMatriz[z+1][x] == 0){
				playerList[1].zPlayer+=speedE;
				lastMove = direction;
			}else if(mapMatriz[z+1][x] == 2){
				bomb b;
				setBomb(b, 1);
				enemyScape(lastMove);
				enemyWait = true;
			}
			
		}
	}
}

bool checkLines(){
	double x1 = playerList[1].xPlayer;//inimigo
	double z1 = playerList[1].zPlayer;//inimigo
	double x2 = playerList[0].xPlayer;//player
	double z2 = playerList[0].zPlayer;//player
	int locX, locZ;
	
	if(x1 == x2){
		for(int i = 1; i < matrizSize-1; i++){
			locX = (int)x1+recoil;
			locZ = (int)i;
			if(mapMatriz[locZ][locX] == 0){
				return true;
			}else{
				continue;
			}
		}
	}else if(z1 == z2){
		for(int i = 1; i < matrizSize-1; i++){
			locX = (int)i;
			locZ = (int)z1+recoil;
			if(mapMatriz[locZ][locX] == 0){
				return true;
			}else{
				continue;
			}
		}
	}
	return false;
	
}

double distanceManhattan(double x1, double z1, double x2, double z2){
    double distanciaX = x2 - x1;
    double distanciaZ = z2 - z1;
    double disTotal = distanciaX + distanciaZ;
    if(disTotal < 0){
		disTotal*=-1;
	}
    return disTotal;
}

bool playerCollision(double x, double z){
	double xPlayer = playerList[0].xPlayer;
	double zPlayer = playerList[0].zPlayer;
	
	if(x == xPlayer && z == zPlayer){
		return true;
	}
	
	return false;
}

bool enemyCollision(double x, double z){
	double xEnemy = playerList[1].xPlayer;
	double zEnemy = playerList[1].zPlayer;
	
	if(x == xEnemy && z == zEnemy){
		return true;
	}
	
	return false;
}

//inimigo irá fugir da bomba na direção oposta 
void enemyScape(int direction){
	int x;
	int z;
	
	if(direction == 3){//foi para baixo
		while(true){
			playerList[1].zPlayer-=playerList[1].speed;//vai para cima
			x = (int)playerList[1].xPlayer+recoil;
			z = (int)playerList[1].zPlayer+recoil;
			if(mapMatriz[z][x-1] == 0){//olhar para a esquerda
				playerList[1].xPlayer-=playerList[1].speed;
				lastMove = 0;
				break;
			}else if(mapMatriz[z][x+1] == 0){
				playerList[1].xPlayer+=playerList[1].speed;
				lastMove = 1;
				break;
			}else{
				continue;
			}
		}
		
	}else if(direction == 2){//foi para cima
		while(true){
			playerList[1].zPlayer+=playerList[1].speed;//vai para baixo
			x = (int)playerList[1].xPlayer+recoil;
			z = (int)playerList[1].zPlayer+recoil;
			if(mapMatriz[z][x-1] == 0){//olhar para a esquerda
				playerList[1].xPlayer-=playerList[1].speed;
				lastMove = 0;
				break;
			}else if(mapMatriz[z][x+1] == 0){//olhar para a direita
				playerList[1].xPlayer+=playerList[1].speed;
				lastMove = 1;
				break;
			}else{
				continue;
			}
		}
	}else if(direction == 1){//foi para direita
		while(true){
			playerList[1].xPlayer-=playerList[1].speed;//vai para esquerda
			x = (int)playerList[1].xPlayer+recoil;
			z = (int)playerList[1].zPlayer+recoil;
			if(mapMatriz[z+1][x] == 0){//olhar para a baixo
				playerList[1].zPlayer+=playerList[1].speed;
				lastMove = 3;
				break;
			}else if(mapMatriz[z-1][x] == 0){//olhar para a cima
				playerList[1].zPlayer-=playerList[1].speed;
				lastMove = 2;
				break;
			}else{
				continue;
			}
		}
	}else if(direction == 0){//foi para esquerda
		while(true){
			playerList[1].xPlayer+=playerList[1].speed;//vai para direita
			x = (int)playerList[1].xPlayer+recoil;
			z = (int)playerList[1].zPlayer+recoil;
			if(mapMatriz[z+1][x] == 0){//olhar para a baixo
				playerList[1].zPlayer+=playerList[1].speed;
				lastMove = 3;
				break;
			}else if(mapMatriz[z-1][x] == 0){//olhar para a cima
				playerList[1].zPlayer-=playerList[1].speed;
				lastMove = 2;
				break;
			}else{
				continue;
			}
		}
	}
}

