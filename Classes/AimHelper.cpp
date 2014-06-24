//
//  AimHelper.cpp
//  t06
//
//  Created by 野口一也 on 2014/06/24.
//
//

#include "AimHelper.h"

void AimHelper::draw(Renderer *renderer, const Mat4 &transform, bool transformUpdated)
{
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(AimHelper::onDraw, this, transform, transformUpdated);
    renderer->addCommand(&_customCommand);
}

void AimHelper::onDraw(const Mat4 &transform, bool transformUpdated)
{
    auto director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    DrawPrimitives::setDrawColor4F(0.8f, 1.0f, 0.76f, 1.0f);
	glLineWidth(6.0f);
    DrawPrimitives::drawLine(_origin, _origin + (_direction * (_length * 1.2f)) );
    
    //end draw
    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}