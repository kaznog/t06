#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    _winSize = Director::getInstance()->getWinSize();
    auto drawNode = DrawNode::create();
    drawNode->drawSegment(Point(0, _winSize.height/2), Point(_winSize.width, _winSize.height/2), 1, Color4F(1.0f, 1.0f, 1.0f, 1.0f));
    drawNode->drawSegment(Point(_winSize.width/2, _winSize.height), Point(_winSize.width/2, 0), 1, Color4F(1.0f, 1.0f, 1.0f, 1.0f));
    
    Point centerPoint = Point(_winSize.width/2, _winSize.height/2);
    float R = 128;
    std::vector<Color4F> colors = {
        Color4F(1.0f, 0.0f, 0.0f, 0.6f), Color4F(0.0f, 1.0f, 0.0f, 0.6f),
        Color4F(0.0f, 0.0f, 1.0f, 0.6f), Color4F(0.5f, 0.5f, 0.0f, 0.6f),
        Color4F(0.0f, 0.5f, 0.5f, 0.6f), Color4F(0.5f, 0.5f, 0.5f, 0.5f),
    };
    for (float angles = 0, i = 0; angles < 360; angles += 60, i++) {
        float x = R * cos(CC_DEGREES_TO_RADIANS(angles));
        float y = R * sin(CC_DEGREES_TO_RADIANS(angles));
        drawNode->drawDot(centerPoint + Point(x, y), R/2, colors[i]);
        auto label = LabelTTF::create("angle:" + std::to_string((int)(angles)), "Arial", 24);
        label->setPosition(centerPoint + Point(x, y));
        drawNode->addChild(label);
    }
    this->addChild(drawNode, 1, 1);

    //    auto label = LabelTTF::create("angle(000)", "Arial", 48);
    //    label->setColor(Color3B::WHITE);
    //    label->setTag(TAG_LABEL);
    //    this->addChild(label, TAG_LABEL, TAG_LABEL);
    //    label->setPosition(Point(winSize.width/2, winSize.height/2));
    //    enableTouchEvent(true);
    return true;
}

#pragma mark touch events
void HelloWorld::enableTouchEvent(bool enabled)
{
    if (this->_touchListener != nullptr) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(this->_touchListener);
        this->_touchListener = nullptr;
    }
    if (enabled) {
        this->_touchListener = EventListenerTouchOneByOne::create();
        _touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
        _touchListener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
        _touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
        _touchListener->onTouchCancelled = CC_CALLBACK_2(HelloWorld::onTouchCancelled, this);
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);
    }
}

bool HelloWorld::onTouchBegan(Touch *touch, Event *event)
{
    Point location = touch->getLocation();
    float angle = CC_RADIANS_TO_DEGREES( atan2((location.x - _winSize.width/2), (location.y - _winSize.height/2)) );
    auto circle = this->getChildByTag(TAG_CIRCLE);
    circle->setPosition(Point(cosf(angle) * 200, sinf(angle) * 200));
    auto label = (LabelTTF*)this->getChildByTag(TAG_LABEL);
    label->setString("angle(" + std::to_string(angle) + ")");
    circle->setVisible(true);
    
    return true;
}

void HelloWorld::onTouchMoved(Touch *touch, Event *event)
{
    Point location = touch->getLocation();
    float angle = CC_RADIANS_TO_DEGREES( atan2((location.x - _winSize.width/2), (location.y - _winSize.height/2)) );
    auto circle = this->getChildByTag(TAG_CIRCLE);
    circle->setPosition(Point(cosf(angle) * 200, sinf(angle) * 200));
    auto label = (LabelTTF*)this->getChildByTag(TAG_LABEL);
    label->setString("angle(" + std::to_string(angle) + ")");
    circle->setVisible(true);
}

void HelloWorld::onTouchEnded(Touch *touch, Event *event)
{
    Point location = touch->getLocation();
    float angle = CC_RADIANS_TO_DEGREES( atan2((location.x - _winSize.width/2), (location.y - _winSize.height/2)) );
    auto circle = this->getChildByTag(TAG_CIRCLE);
    circle->setPosition(Point(cosf(angle) * 200, sinf(angle) * 200));
    auto label = (LabelTTF*)this->getChildByTag(TAG_LABEL);
    label->setString("angle(" + std::to_string(angle) + ")");
    circle->setVisible(true);
}

void HelloWorld::onTouchCancelled(Touch *touch, Event *event)
{
    Point location = touch->getLocation();
    float angle = CC_RADIANS_TO_DEGREES( atan2((location.x - _winSize.width/2), (location.y - _winSize.height/2)) );
    auto circle = this->getChildByTag(TAG_CIRCLE);
    circle->setPosition(Point(cosf(angle) * 200, sinf(angle) * 200));
    auto label = (LabelTTF*)this->getChildByTag(TAG_LABEL);
    label->setString("angle(" + std::to_string(angle) + ")");
    circle->setVisible(true);
}

#pragma mark -


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
