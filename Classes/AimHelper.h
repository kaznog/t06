//
//  AimHelper.h
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#ifndef __t06__AimHelper__
#define __t06__AimHelper__

#include "cocos2d.h"

USING_NS_CC;

class AimHelper : public Node
{
public:
    CC_SYNTHESIZE(Point, _origin, Origin);
    CC_SYNTHESIZE(Point, _direction, Direction);
    CC_SYNTHESIZE(float, _length, Length);
    
    CREATE_FUNC(AimHelper);
    virtual void draw(Renderer *renderer, const Mat4 &transform, bool transformUpdated) override;
protected:
    void onDraw(const Mat4 &transform, bool transformUpdated);
    CustomCommand _customCommand;
};

#endif /* defined(__t06__AimHelper__) */
