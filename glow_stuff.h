/*
===================================


           Racing Stuff
              Troid92


===================================
*/


// System
extern const float PI;
extern const float dtr;

float nmGetAngle(float x1, float y1, float x2, float y2)
{
    if (x2 == x1)
    {
        if ( y1 > y2 ) return 270;
        if ( y1 < y2 ) return 90;
    }
    if (y1 == y2)
    {
        if ( x2 < x1 ) return 180;
        if ( x2 > x1 ) return 0;
    }

    float angle = atan2(y2-y1, x2-x1);
    angle *= dtr;

    if ( angle < 0 ) {
        angle += 360;
    }

    return angle;
}













GLuint nmLoadImage(char* filename, bool transparent)
{
    GLuint texture;
    bool wrap = true;

    corona::Image* image = corona::OpenImage(filename, corona::PF_R8G8B8);
    if (!image) {
        // error!
        return 0;
    }

    corona::FlipImage(image, 45);

    int width  = image->getWidth();
    int height = image->getHeight();
    unsigned char *data = (unsigned char*)image->getPixels();

    // we're guaranteed that the first eight bits of every pixel is red,
    // the next eight bits is green, and so on...

    unsigned char *imageBuf = new unsigned char[width*height*4];

    for(int i=0, j=0; i<width*height*3; i+=3, j+=4 )
    {
        imageBuf[j] = 255;
        imageBuf[j+1] = 255;
        imageBuf[j+2] = 255;
        imageBuf[j+3] = data[i];
    }


    // --------------------------------------
    // Setup the OpenGL texture with the data
    // --------------------------------------


    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_NEAREST);//LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//LINEAR );


    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, width, height,
                       GL_RGBA, GL_UNSIGNED_BYTE, imageBuf);
    delete[] imageBuf;
    delete image;
    return texture;

}



class Particle
{
    private:
    float x, y, xspeed, yspeed, red, green, blue, colorAnim;
    int destx, desty;
    public:
    Particle();
    void Update(bool );
    float GetRed();
    float GetGreen();
    float GetBlue();
    float Getx();
    float Gety();
    int GetDestx();
    int GetDesty();
    void SetDest(int, int);
};

Particle::Particle()
{
    destx = desty = 0;
    x = rand()%641;
    y = rand()%481;
    xspeed = rand()%10-5;
    yspeed = rand()%10-5;
    colorAnim = (rand()%360)*dtr;
    red = sin(colorAnim) >= 0 ? sin(colorAnim) : -sin(colorAnim);
    green = sin(colorAnim+1.04719755f) >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    blue = sin(colorAnim+2.0943951f) >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);
}

void Particle::Update(bool GoToDest)
{
    colorAnim += .01f;
    red = sin(colorAnim) >= 0 ? sin(colorAnim) : -sin(colorAnim);
    green = sin(colorAnim+1.04719755f) >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    blue = sin(colorAnim+2.0943951f) >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);

    if (GoToDest)
    {
        xspeed += (destx-x)/320.0f;
        yspeed += (desty-y)/240.0f;
        xspeed *= .98f;
        yspeed *= .98f;
    }
    else
    {
        xspeed += ((rand()%101)/400.0f)-.125f;
        yspeed += ((rand()%101)/400.0f)-.125f;
    }

    x += xspeed;
    y += yspeed;
    if (x < 0)
    {
        x = 0;
        xspeed = -xspeed;
    }
    if (x > 640)
    {
        x = 640;
        xspeed = -xspeed;
    }
    if (y < 0)
    {
        y = 0;
        yspeed = -yspeed;
    }
    if (y > 448)
    {
        y = 448;
        yspeed = -yspeed;
    }
}

float Particle::GetRed() { return red; }

float Particle::GetGreen() { return green; }

float Particle::GetBlue() { return blue; }

float Particle::Getx() { return x; }

float Particle::Gety() { return y; }

int Particle::GetDestx() { return destx; }

int Particle::GetDesty() { return desty; }

void Particle::SetDest(int newx, int newy)
{
    destx = newx;
    desty = newy;
}


class ParticleMap
{
    private:
    float colorAnim;
    unsigned int particleNum;
    Particle* theParticles;
    GLuint particleTex;
    public:
    ParticleMap();
    ~ParticleMap();
    void StepEvent();
    void DrawEvent();
};

ParticleMap::ParticleMap()
{
    particleTex = nmLoadImage("particle.png", true);


    particleNum = 500;


    colorAnim = 0;

    int width, height;
    unsigned char *data;
    corona::Image* image = corona::OpenImage("theMap.glow", corona::PF_R8G8B8);
    unsigned char error[3] = { 0, 0, 0 };
    if (!image) {
        width  = 1;
        height = 1;
        data = error;
    }
    else
    {
        corona::FlipImage(image, 45);

        width  = image->getWidth();
        height = image->getHeight();
        data = (unsigned char*)image->getPixels();
    }

    unsigned int blackPixels = 0;
    for (int i = 0; i < width*height; i += 1)
    {
        if (data[i*3] == 0) blackPixels += 1;
    }

    float pixelSkip = (float)blackPixels / particleNum;

    theParticles = new Particle[particleNum];
    unsigned int particleCount = 0;
    float blackPixelCount = 0;

    for (int i = 0; i < width*height*3; i += 3)
    {
        if (data[i] == 0)
        {
            if (blackPixelCount >= pixelSkip)
            {
                theParticles[particleCount].SetDest((i/3)%width,(i/3)/width);
                particleCount += 1;
                blackPixelCount = 0;
            }
            blackPixelCount += 1;
        }
    }

    delete image;

    unsigned int randParticle;
    for (int i = 0; i < particleNum; i++)
    {
        if (theParticles[i].GetDestx() == 0 && theParticles[i].GetDesty() == 0)
        {
            randParticle = rand()%particleNum;
            theParticles[i].SetDest(theParticles[randParticle].GetDestx(),theParticles[randParticle].GetDesty());
        }
    }
}

ParticleMap::~ParticleMap()
{
    delete[] theParticles;
    glDeleteTextures(1,&particleTex);
}

void ParticleMap::StepEvent()
{
    bool GoToDest = spaceDown;
    for (int i = 0; i < particleNum; i++)
    {
        theParticles[i].Update(GoToDest);
    }
}

void ParticleMap::DrawEvent()
{
    colorAnim += .01f;
    float red = 1-sin(colorAnim);// >= 0 ? sin(colorAnim) : -sin(colorAnim);
    float green = 1-sin(colorAnim+1.04719755f);// >= 0 ? sin(colorAnim+1.04719755f) : -sin(colorAnim+1.04719755f);
    float blue = 1-sin(colorAnim+2.0943951f);// >= 0 ? sin(colorAnim+2.0943951f) : -sin(colorAnim+2.0943951f);
    glBindTexture(GL_TEXTURE_2D, particleTex);
    glBegin(GL_QUADS);
    for (int i = 0; i < particleNum; i++)
    {
    glColor3f(theParticles[i].GetRed(),
              theParticles[i].GetGreen(),
              theParticles[i].GetBlue());
        glTexCoord2f(0.0f,0.0f); glVertex2f(theParticles[i].Getx()-32,theParticles[i].Gety()-32);
        glTexCoord2f(1.0f,0.0f); glVertex2f(theParticles[i].Getx()+32,theParticles[i].Gety()-32);
        glTexCoord2f(1.0f,1.0f); glVertex2f(theParticles[i].Getx()+32,theParticles[i].Gety()+32);
        glTexCoord2f(0.0f,1.0f); glVertex2f(theParticles[i].Getx()-32,theParticles[i].Gety()+32);
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

