//
//  UVSprite.h
//  HelloCpp
//
//  Created by neo on 15-1-10.
//
//

#ifndef __HelloCpp__UVSprite__
#define __HelloCpp__UVSprite__

#include "cocos2d.h"

class UVSprite : public cocos2d::CCSprite
{
    
    virtual bool initWithTexture(cocos2d::CCTexture2D *pTexture, const cocos2d::CCRect& rect, bool rotated);
    virtual void draw();
    virtual void update(float dt);
    virtual void onEnter();
    virtual void onExit();
    
    bool _AutoScrollU = true;
    float _AutoScrollSpeedU =0;
    bool _AutoScrollV = false;
    float _AutoScrollSpeedV=0;
    
    
    
    float _AutoScrollCountU=0;
    float _AutoScrollCountV=0;
    
    GLuint _uniformOffset;
    
    static UVSprite* create();
    
    ~UVSprite();
    void listenBackToForeground(cocos2d::CCObject *obj);
    void loadShaderVertex(const char *vert, const char *frag);
public:
    
    UVSprite()
    :_uniformOffset(0)
    {}
    static UVSprite* createWithSpriteFrameName(const char *pszSpriteFrameName);
    static UVSprite* create(const char *pszFileName);
    
    void setAutoScrollU(bool scroll)
    {
        _AutoScrollU = scroll;
    }
    
    void setAutoScrollV(bool scroll)
    {
        _AutoScrollV = scroll;
    }
    void setScrollSpeedU(float speed)
    {
        _AutoScrollSpeedU = speed;
    }
    
    
    void setScrollSpeedV(float speed)
    {
        _AutoScrollSpeedV = speed;
    }
    
    bool isAutoScrollU(){return _AutoScrollU;}
    bool isAutoScrollV(){return _AutoScrollV;}
    float getScrollSpeedV()
    {
        return _AutoScrollSpeedV;
    }
    float getScrollSpeedU()
    {
        return _AutoScrollSpeedU;
    }
};

#endif /* defined(__HelloCpp__UVSprite__) */
