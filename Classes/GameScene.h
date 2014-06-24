//
//  GameScene.h
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#ifndef __t06__GameScene__
#define __t06__GameScene__

#include "cocos2d.h"
#include "Constants.h"
#include "Field.h"

USING_NS_CC;

class GameScene :  public Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    virtual void update(float delta);
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);
private:
    Field* _field;
};

#endif /* defined(__t06__GameScene__) */
