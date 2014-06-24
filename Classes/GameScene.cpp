//
//  GameScene.cpp
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#include "GameScene.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
    auto scene = Scene::create();
    auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}


bool GameScene::init()
{
    if (!Layer::init()) {
        return false;
    }
    auto director = Director::getInstance();
    Size visibleSize = director->getVisibleSize();
    Point origin = director->getVisibleOrigin();
    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));
    
	closeItem->setPosition(Point(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                 origin.y + closeItem->getContentSize().height/2));
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Point::ZERO);
    this->addChild(menu, 1);
    
    
    _field = Field::create();
    this->addChild(_field, 10);
    
    
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
    return true;
}

void GameScene::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void GameScene::update(float delta)
{
    _field->update(delta);
}