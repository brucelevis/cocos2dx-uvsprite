//
//  UVSprite.cpp
//  HelloCpp
//
//  Created by neo on 15-1-10.
//
//

#include "UVSprite.h"

const char* frag_shader = "											\n\
#ifdef GL_ES								\n\
precision lowp float;						\n\
#endif										\n\
\n\
varying vec4 v_fragmentColor;				\n\
varying vec2 v_texCoord;					\n\
uniform vec2 texOffset;                 \n\
uniform sampler2D CC_Texture0;				\n\
\n\
void main()									\n\
{											\n\
//vec2 texcoord = vec2(v_texCoord.x + texOffsetX,v_texCoord.y + texOffsetY); \n\
vec2 texcoord = mod(texOffset+v_texCoord,1.0);   \n\
gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, texcoord);			\n\
}											\n\
";

using namespace cocos2d;

UVSprite::~UVSprite()
{
    CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, EVENT_COME_TO_FOREGROUND);
}
void UVSprite::draw()
{
    CC_PROFILER_START_CATEGORY(kCCProfilerCategorySprite, "CCSprite - draw");
    
    CCAssert(!m_pobBatchNode, "If CCSprite is being rendered by CCSpriteBatchNode, CCSprite#draw SHOULD NOT be called");
    
    getShaderProgram()->use();
    getShaderProgram()->setUniformsForBuiltins();
    
    ccGLBlendFunc( m_sBlendFunc.src, m_sBlendFunc.dst );
    
    getShaderProgram()->setUniformLocationWith2f(_uniformOffset, _AutoScrollCountU, _AutoScrollCountV);

    //绑定纹理贴图
    ccGLBindTexture2D( m_pobTexture->getName() );
    ccGLEnableVertexAttribs( kCCVertexAttribFlag_PosColorTex );
    
#define kQuadSize sizeof(m_sQuad.bl)
#ifdef EMSCRIPTEN
    long offset = 0;
    setGLBufferData(&m_sQuad, 4 * kQuadSize, 0);
#else
    long offset = (long)&m_sQuad;
#endif // EMSCRIPTEN
    
    
    // 设置渲染坐标(x,y)
    int diff = offsetof( ccV3F_C4B_T2F, vertices);
    glVertexAttribPointer(kCCVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*) (offset + diff));
    
    // 设置纹理坐标(u,v)
    diff = offsetof( ccV3F_C4B_T2F, texCoords);
    glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));
    
    // 设置顶点颜色
    diff = offsetof( ccV3F_C4B_T2F, colors);
    glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));
    
    //渲染矩形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CHECK_GL_ERROR_DEBUG();
    
    
#if CC_SPRITE_DEBUG_DRAW == 1
    // draw bounding box
    CCPoint vertices[4]={
        ccp(m_sQuad.tl.vertices.x,m_sQuad.tl.vertices.y),
        ccp(m_sQuad.bl.vertices.x,m_sQuad.bl.vertices.y),
        ccp(m_sQuad.br.vertices.x,m_sQuad.br.vertices.y),
        ccp(m_sQuad.tr.vertices.x,m_sQuad.tr.vertices.y),
    };
    ccDrawPoly(vertices, 4, true);
#elif CC_SPRITE_DEBUG_DRAW == 2
    // draw texture box
    CCSize s = this->getTextureRect().size;
    CCPoint offsetPix = this->getOffsetPosition();
    CCPoint vertices[4] = {
        ccp(offsetPix.x,offsetPix.y), ccp(offsetPix.x+s.width,offsetPix.y),
        ccp(offsetPix.x+s.width,offsetPix.y+s.height), ccp(offsetPix.x,offsetPix.y+s.height)
    };
    ccDrawPoly(vertices, 4, true);
#endif // CC_SPRITE_DEBUG_DRAW
    
    CC_INCREMENT_GL_DRAWS(1);
    
    CC_PROFILER_STOP_CATEGORY(kCCProfilerCategorySprite, "CCSprite - draw");

}
void UVSprite::update(float dt)
{
    CCSprite::update(dt);
    
    //更新u
    if(_AutoScrollU)
    {
        _AutoScrollCountU += dt * _AutoScrollSpeedU;
    }
    
    //更新v
    if (_AutoScrollV) {
        _AutoScrollCountV += dt * _AutoScrollSpeedV;
    }
    
    //如果超出范围从0开始
    if (_AutoScrollCountU > 1 || _AutoScrollCountU < -1) {
        _AutoScrollCountU = 0;
    }
    
    if (_AutoScrollCountV > 1 || _AutoScrollCountV < -1) {
        _AutoScrollCountV = 0;
    }
}

void UVSprite::onEnter()
{
    CCSprite::onEnter();
    scheduleUpdate();
}

void UVSprite::onExit()
{
    unscheduleUpdate();
    CCSprite::onExit();
}

UVSprite* UVSprite::create()
{
    UVSprite *pSprite = new UVSprite();
    if (pSprite && pSprite->init())
    {
        pSprite->autorelease();
        return pSprite;
    }
    CC_SAFE_DELETE(pSprite);
    return NULL;
}
UVSprite* UVSprite::create(const char *pszFileName)
{
    UVSprite *pobSprite = new UVSprite();
    if (pobSprite && pobSprite->initWithFile(pszFileName))
    {
        pobSprite->autorelease();
        return pobSprite;
    }
    CC_SAFE_DELETE(pobSprite);
    return NULL;
}
bool UVSprite::initWithTexture(cocos2d::CCTexture2D *pTexture, const cocos2d::CCRect& rect, bool rotated)
{
    if (CCNodeRGBA::init())
    {
        m_pobBatchNode = NULL;
        
        m_bRecursiveDirty = false;
        setDirty(false);
        
        m_bOpacityModifyRGB = true;
        
        m_sBlendFunc.src = CC_BLEND_SRC;
        m_sBlendFunc.dst = CC_BLEND_DST;
        
        m_bFlipX = m_bFlipY = false;
        
        // default transform anchor: center
        setAnchorPoint(ccp(0.5f, 0.5f));
        
        // zwoptex default values
        m_obOffsetPosition = CCPointZero;
        
        m_bHasChildren = false;
        
        // clean the Quad
        memset(&m_sQuad, 0, sizeof(m_sQuad));
        
        // Atlas: Color
        ccColor4B tmpColor = { 255, 255, 255, 255 };
        m_sQuad.bl.colors = tmpColor;
        m_sQuad.br.colors = tmpColor;
        m_sQuad.tl.colors = tmpColor;
        m_sQuad.tr.colors = tmpColor;
        
        // shader program
        // shader program
        CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                                      callfuncO_selector(UVSprite::listenBackToForeground),
                                                                      EVENT_COME_TO_FOREGROUND,
                                                                      NULL);
        loadShaderVertex(ccPositionTextureColor_vert, frag_shader);
        
        // update texture (calls updateBlendFunc)
        setTexture(pTexture);
        setTextureRect(rect, rotated, rect.size);
        
        // by default use "Self Render".
        // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
        setBatchNode(NULL);
        
        return true;
    }
    else
    {
        return false;
    }

}

void UVSprite::loadShaderVertex(const char *vert, const char *frag)
{
    CCGLProgram *shader = new CCGLProgram();
    shader->initWithVertexShaderByteArray(vert, frag);
    
    shader->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    shader->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
    shader->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
    
    shader->link();
    
    shader->updateUniforms();
    
    _uniformOffset = glGetUniformLocation(shader->getProgram(), "texOffset");
    
    this->setShaderProgram(shader);
    
    shader->release();
}

void UVSprite::listenBackToForeground(CCObject *obj)
{
    this->setShaderProgram(NULL);
    loadShaderVertex(ccPositionTextureColor_vert, frag_shader);
}


UVSprite* UVSprite::createWithSpriteFrameName(const char *pszSpriteFrameName)
{
    CCSpriteFrame *pSpriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(pszSpriteFrameName);
    
#if COCOS2D_DEBUG > 0
    char msg[256] = {0};
    sprintf(msg, "Invalid spriteFrameName: %s", pszSpriteFrameName);
    CCAssert(pSpriteFrame != NULL, msg);
#endif
    
    UVSprite *pobSprite = new UVSprite();
    if (pSpriteFrame && pobSprite && pobSprite->initWithSpriteFrame(pSpriteFrame))
    {
        pobSprite->autorelease();
        return pobSprite;
    }
    CC_SAFE_DELETE(pobSprite);
    return NULL;
}

