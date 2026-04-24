#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

//定数定義でマジックナンバーを防止
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
constexpr float FOV = 400.0f;
constexpr float PI = 3.14159265f;

//0除算防止用の最小値z
constexpr float Z_MIN_LIMIT = 0.1f;

//3Dオブジェクトの数値
constexpr float CUBE_HALF_SIZE = 5.0f;  //立方体サイズの半分
constexpr float AXIS_LENGTH = 20.0f;    //軸の長さ
constexpr float CAMERA_OFFSET = 40.0f;   //カメラからの奥行き

//操作・システムの定数
constexpr float ROTATE_SPEED = 10.0f;    //キー入力時の回転速度
constexpr Uint32 FRAME_DELAY = 16;      //待機時間（60FPS）

//ディグリーから弧度法に変換するマクロ
#define DEGREE(_kakudo) (_kakudo*(PI / 180.0f))

//1.構造体の定義
struct Vector3 {
    float x, y, z;
};

struct Vector2 {
    float x, y;
};


//
//2.関数の定義------------------------------------------------------------------------------------------------------------
//
Vector2 ProjectToScreen(Vector3 point, float fov, int screenWidth, int screenHeight) {
    Vector2 screenPoint;
    if (point.z == 0.0f) point.z = Z_MIN_LIMIT;    //0除算を防ぐ

    //zで割ることで奥行きを表現して画面中央を原点にずらす。
    screenPoint.x = (point.x / point.z) * fov + (screenWidth / 2.0f);
    screenPoint.y = (point.y / point.z) * fov + (screenHeight / 2.0f);
    return screenPoint;
}

//
//追加。z軸が視認不可能だったため回転関数を追加
//
Vector3 RotateY(Vector3 p, float angle) {
    Vector3 rotated;
    rotated.x = p.x * cos(angle) - p.z * sin(angle);
    rotated.y = p.y;
    rotated.z = p.x * sin(angle) + p.z * cos(angle);

    return rotated;
}

Vector3 RotateX(Vector3 p, float angle) {
    Vector3 rotated;
    rotated.x = p.x;
    rotated.y = p.y * cos(angle) - p.z * sin(angle);
    rotated.z = p.y * sin(angle) + p.z * cos(angle);

    return rotated;
}


//
//3.main関数
//
int main(int argc, char* argv[]){
    
    
    
    
    
    
    
    //初期化部分-----------------------------------------------------------------------------------------------------------
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    SDL_Window* window = SDL_CreateWindow("3D to 2D Projection",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WIDTH,HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    //構造を使っててを作成
    //Vector3 myPoint = {10.0f, 10.0f, 5.0f};   //これは画面外に描画されたから却下
    //Vector3 myPoint = {2.0f,2.0f,5.0f};   //これでは1つの点しか打てない
    //立方体の点の座標定義
    Vector3 myPoint[8] = {
    // 前面の4頂点 (手前側)
    {-CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE}, // 0: 左上
    { CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE}, // 1: 右上
    { CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE}, // 2: 右下
    {-CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE}, // 3: 左下
    
    // 背面の4頂点 (奥側)
    {-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE}, // 4: 左上
    { CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE}, // 5: 右上
    { CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE}, // 6: 右下
    {-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE}  // 7: 左下
    };


    //原点とそこから伸びる線の終着点を定める
    Vector3 genten = {0.0f,0.0f,0.0f};
    Vector3 hajikko[3] = {
        {AXIS_LENGTH, 0.0f, 0.0f},   //x軸の終端
        {0.0f, -AXIS_LENGTH, 0.0f},  //y軸の終端
        {0.0f, 0.0f, AXIS_LENGTH}    //z軸の終端  
    };

    //関数を渡して2D座標に変換する(一旦宣言しとく)
    Vector2 projected;
    Vector2 screengenten;
    Vector2 screenhajikko[3];

    //回転後の座標（3次元）
    Vector3 tempAxis;
    Vector3 rotateAxis;
    //立方体回転用
    Vector3 tempmypoint[8];
    Vector3 rotatemypoint[8];

    //追加　現在の回転角度
    float currentYAngle = 0.0f;
    float currentXAngle = 0.0f;
    //ループ内で回転角度をディグリーから弧度法に最初に変換して入れる用変数
    float radY;
    float radX;

    //立方体の面のインデックス配列
    const int indices[36] = {
        //前面
        0,1,2,  0,2,3,
        //背面
        4,5,6,  4,6,7,
        //左面
        0,3,7,  0,4,7,
        //右面
        1,2,6,  1,5,6,
        //下面
        2,3,7,  2,6,7,
        //上面
        0,1,5,  0,4,5    
    };

    //立方体の色を定義する。
    SDL_Vertex vertices[8];
    SDL_Color faceColor = {255,255,255,128};
    for(int i = 0; i < 8; i++) {
        vertices[i].color = faceColor;
        vertices[i].tex_coord = {0.0f, 0.0f};
    }

    //軸用の色を配列で定義しておく
    SDL_Color axisColor[3] = {
        {255, 0, 0, 255}, //赤x
        {0, 255, 0, 255}, //緑y
        {0, 0, 255, 255}, //青z
    };

    //半透明を有効にする
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);



    
    
    
    bool isRunning = true;
    SDL_Event event;
    //メインループ-----------------------------------------------------------------------------------------------------------
    while (isRunning) {
        //xボタンでループ脱出
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                isRunning = false;
            }
            //追加。キーボード受付
            if(event.type == SDL_KEYDOWN) {
                // 左矢印キーで角度減らす
                if(event.key.keysym.sym == SDLK_LEFT){
                    currentYAngle -= ROTATE_SPEED;
                }
                // 右矢印キーで角度増加する
                if(event.key.keysym.sym == SDLK_RIGHT){
                    currentYAngle += ROTATE_SPEED;
                }
                if(event.key.keysym.sym == SDLK_UP){
                    currentXAngle -= ROTATE_SPEED;
                }
                // 右矢印キーで角度増加する
                if(event.key.keysym.sym == SDLK_DOWN){
                    currentXAngle += ROTATE_SPEED;
                }
            }
        }

        //描画処理
        //描画クリア
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        //角度の正規化
        currentXAngle = std::fmod(currentXAngle, 360.0f);
        currentYAngle = std::fmod(currentYAngle, 360.0f);

        if(currentXAngle < 0.0f) currentYAngle += 360.0f;
        if(currentYAngle < 0.0f) currentXAngle += 360.0f;

        //角度を計算
        radY = DEGREE(currentYAngle);
        radX = DEGREE(currentXAngle);

        //原点計算
        Vector3 tempgenten = RotateY(genten,radY);
        Vector3 rotategenten = RotateX(tempgenten,radX);
        rotategenten.z += CAMERA_OFFSET;

        //追記。原点と終端を繋いだ線を描画して軸を作成する。
        screengenten = ProjectToScreen(rotategenten, FOV, WIDTH, HEIGHT);
        for(int i = 0 ; i < 3 ; i++){
            tempAxis = RotateY(hajikko[i], radY);
            rotateAxis = RotateX(tempAxis, radX);
            rotateAxis.z += CAMERA_OFFSET;
            screenhajikko[i] = ProjectToScreen(rotateAxis, FOV, WIDTH, HEIGHT);

            // axisColorから取り出して色を設定
            SDL_SetRenderDrawColor(renderer, axisColor[i].r, axisColor[i].g, axisColor[i].b, axisColor[i].a);
            SDL_RenderDrawLine(renderer, screengenten.x, screengenten.y, screenhajikko[i].x, screenhajikko[i].y);
        
        }
        

        
        //指定した座標から立体を形成
        for(int i = 0 ; i < 8 ; i++){
            //関数を渡して2D座標に変換する処理
            tempmypoint[i] = RotateY(myPoint[i],radY);
            rotatemypoint[i] = RotateX(tempmypoint[i],radX);
            rotatemypoint[i].z += CAMERA_OFFSET;
            projected = ProjectToScreen(rotatemypoint[i], FOV, WIDTH, HEIGHT);

            vertices[i].position.x = projected.x;
            vertices[i].position.y = projected.y;
        }
        //まとめて描画
        SDL_RenderGeometry(renderer, nullptr, vertices, 8, indices, 36);

        //裏で書いたものを画面に表示（更新処理）
        SDL_RenderPresent(renderer);

        //CPUを休ませる（手動で約60fpsでの更新を行う）
        SDL_Delay(FRAME_DELAY);
    }

    //終了処理
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0; 
}