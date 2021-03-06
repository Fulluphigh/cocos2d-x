//
// Accelerometer + Chipmunk physics + multi touches example
// a cocos2d example
// http://www.cocos2d-x.org
//

#include "ChipmunkTest.h"


enum {
    kTagParentNode = 1,
};

enum {
    Z_PHYSICS_DEBUG = 100,
};

// callback to remove Shapes from the Space

ChipmunkTestLayer::ChipmunkTestLayer()
{
#if CC_ENABLE_CHIPMUNK_INTEGRATION      
    // enable events
    setTouchEnabled(true);
    setAccelerometerEnabled(true);

    // title
    CCLabelTTF *label = CCLabelTTF::create("Multi touch the screen", "Marker Felt", 36);
    label->setPosition(ccp( VisibleRect::center().x, VisibleRect::top().y - 30));
    this->addChild(label, -1);

    // reset button
    createResetButton();

    // init physics
    initPhysics();

#if 1
    // Use batch node. Faster
    CCSpriteBatchNode *parent = CCSpriteBatchNode::create("Images/grossini_dance_atlas.png", 100);
    _spriteTexture = parent->getTexture();
#else
    // doesn't use batch node. Slower
    _spriteTexture = CCTextureCache::sharedTextureCache()->addImage("Images/grossini_dance_atlas.png");
    CCNode *parent = CCNode::create();
#endif
    addChild(parent, 0, kTagParentNode);

    addNewSpriteAtPosition(ccp(200,200));

    // menu for debug layer
    CCMenuItemFont::setFontSize(18);
    CCMenuItemFont *item = CCMenuItemFont::create("Toggle debug", CC_CALLBACK_1(ChipmunkTestLayer::toggleDebugCallback, this));

    CCMenu *menu = CCMenu::create(item, NULL);
    this->addChild(menu);
    menu->setPosition(ccp(VisibleRect::right().x-100, VisibleRect::top().y-60));

    scheduleUpdate();
#else
    CCLabelTTF *pLabel = CCLabelTTF::create("Should define CC_ENABLE_CHIPMUNK_INTEGRATION=1\n to run this test case",
                                            "Arial",
                                            18);
    CCSize size = CCDirector::sharedDirector()->getWinSize();
    pLabel->setPosition(ccp(size.width/2, size.height/2));
    
    addChild(pLabel);
    
#endif
    
}

void ChipmunkTestLayer::toggleDebugCallback(CCObject* pSender)
{
#if CC_ENABLE_CHIPMUNK_INTEGRATION
    _debugLayer->setVisible(! _debugLayer->isVisible());
#endif
}

ChipmunkTestLayer::~ChipmunkTestLayer()
{
    // manually Free rogue shapes
    for( int i=0;i<4;i++) {
        cpShapeFree( _walls[i] );
    }

    cpSpaceFree( _space );

}

void ChipmunkTestLayer::initPhysics()
{
#if CC_ENABLE_CHIPMUNK_INTEGRATION    
    // init chipmunk
    //cpInitChipmunk();

    _space = cpSpaceNew();

    _space->gravity = cpv(0, -100);

    //
    // rogue shapes
    // We have to free them manually
    //
    // bottom
    _walls[0] = cpSegmentShapeNew( _space->staticBody,
        cpv(VisibleRect::leftBottom().x,VisibleRect::leftBottom().y),
        cpv(VisibleRect::rightBottom().x, VisibleRect::rightBottom().y), 0.0f);

    // top
    _walls[1] = cpSegmentShapeNew( _space->staticBody, 
        cpv(VisibleRect::leftTop().x, VisibleRect::leftTop().y),
        cpv(VisibleRect::rightTop().x, VisibleRect::rightTop().y), 0.0f);

    // left
    _walls[2] = cpSegmentShapeNew( _space->staticBody,
        cpv(VisibleRect::leftBottom().x,VisibleRect::leftBottom().y),
        cpv(VisibleRect::leftTop().x,VisibleRect::leftTop().y), 0.0f);

    // right
    _walls[3] = cpSegmentShapeNew( _space->staticBody, 
        cpv(VisibleRect::rightBottom().x, VisibleRect::rightBottom().y),
        cpv(VisibleRect::rightTop().x, VisibleRect::rightTop().y), 0.0f);

    for( int i=0;i<4;i++) {
        _walls[i]->e = 1.0f;
        _walls[i]->u = 1.0f;
        cpSpaceAddStaticShape(_space, _walls[i] );
    }

    // Physics debug layer
    _debugLayer = CCPhysicsDebugNode::create(_space);
    this->addChild(_debugLayer, Z_PHYSICS_DEBUG);
#endif
}

void ChipmunkTestLayer::update(float delta)
{
    // Should use a fixed size step based on the animation interval.
    int steps = 2;
    float dt = CCDirector::sharedDirector()->getAnimationInterval()/(float)steps;

    for(int i=0; i<steps; i++){
        cpSpaceStep(_space, dt);
    }
}

void ChipmunkTestLayer::createResetButton()
{
    CCMenuItemImage *reset = CCMenuItemImage::create("Images/r1.png", "Images/r2.png", CC_CALLBACK_1(ChipmunkTestLayer::reset, this));

    CCMenu *menu = CCMenu::create(reset, NULL);

    menu->setPosition(ccp(VisibleRect::center().x, VisibleRect::bottom().y + 30));
    this->addChild(menu, -1);
}

void ChipmunkTestLayer::reset(CCObject* sender)
{
    CCScene* s = new ChipmunkAccelTouchTestScene();
    ChipmunkTestLayer* child = new ChipmunkTestLayer();
    s->addChild(child);
    child->release();
    CCDirector::sharedDirector()->replaceScene(s);
    s->release();
}

void ChipmunkTestLayer::addNewSpriteAtPosition(CCPoint pos)
{
#if CC_ENABLE_CHIPMUNK_INTEGRATION    
    int posx, posy;

    CCNode *parent = getChildByTag(kTagParentNode);

    posx = CCRANDOM_0_1() * 200.0f;
    posy = CCRANDOM_0_1() * 200.0f;

    posx = (posx % 4) * 85;
    posy = (posy % 3) * 121;


    int num = 4;
    cpVect verts[] = {
        cpv(-24,-54),
        cpv(-24, 54),
        cpv( 24, 54),
        cpv( 24,-54),
    };

    cpBody *body = cpBodyNew(1.0f, cpMomentForPoly(1.0f, num, verts, cpvzero));

    body->p = cpv(pos.x, pos.y);
    cpSpaceAddBody(_space, body);

    cpShape* shape = cpPolyShapeNew(body, num, verts, cpvzero);
    shape->e = 0.5f; shape->u = 0.5f;
    cpSpaceAddShape(_space, shape);

    CCPhysicsSprite *sprite = CCPhysicsSprite::createWithTexture(_spriteTexture, CCRectMake(posx, posy, 85, 121));
    parent->addChild(sprite);

    sprite->setCPBody(body);
    sprite->setPosition(pos);
#endif
}

void ChipmunkTestLayer::onEnter()
{
    CCLayer::onEnter();
}

void ChipmunkTestLayer::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    //Add a new body/atlas sprite at the touched location
    CCSetIterator it;
    CCTouch* touch;

    for( it = touches->begin(); it != touches->end(); it++) 
    {
        touch = (CCTouch*)(*it);

        if(!touch)
            break;

        CCPoint location = touch->getLocation();

        addNewSpriteAtPosition( location );
    }
}

void ChipmunkTestLayer::didAccelerate(CCAcceleration* pAccelerationValue)
{
    static float prevX=0, prevY=0;

#define kFilterFactor 0.05f

    float accelX = (float) pAccelerationValue->x * kFilterFactor + (1- kFilterFactor)*prevX;
    float accelY = (float) pAccelerationValue->y * kFilterFactor + (1- kFilterFactor)*prevY;

    prevX = accelX;
    prevY = accelY;

    CCPoint v = ccp( accelX, accelY);
    v = ccpMult(v, 200);
    _space->gravity = cpv(v.x, v.y);
}

void ChipmunkAccelTouchTestScene::runThisTest()
{
    CCLayer* pLayer = new ChipmunkTestLayer();
    addChild(pLayer);
    pLayer->release();

    CCDirector::sharedDirector()->replaceScene(this);
}

