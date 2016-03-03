//
//  status.cpp
//  ConnectionsWall
//
//  Created by Lab on 9/25/15.
//
//

#include "status.h"
#include "SignalsManager.h"

Status::Status(){
    mCurStatus = .0;
    isDraw = false;
    SignalsManager::Instance().status.connect(std::bind(&Status::update, this, std::placeholders::_1 ,std::placeholders::_2));
}


void Status::update(float curStatus, bool draw ){
    mCurStatus = curStatus;
    isDraw = draw;
}


void Status::draw(){
    if (isDraw){
        {
            ci::gl::ScopedMatrices push;
            ci::gl::ScopedBlendAlpha alpha;
            ci::gl::translate(ci::vec2(20.f, 230.f));
            {
                ci::gl::ScopedMatrices push;
                ci::gl::ScopedBlendAlpha alpha;
                ci::gl::translate(ci::vec2(4, 3));

                ci::gl::scale(ci::vec2(mCurStatus, 1.));
                ci::gl::ScopedColor color(0,1.0,1,.8);
                ci::gl::drawSolidRect(ci::Rectf(0,0,294,9));
            }
            ci::gl::ScopedColor color(1,1.0,1,.8);
            ci::gl::drawStrokedRect(ci::Rectf(0,0,300,15), 3.f);
        }
    }

    
}