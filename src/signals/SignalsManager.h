//
//  SignalsManager.h
//  ConnectionsWall
//
//  Created by NBartzokas on 8/4/15.
//
//

#pragma once

#include "cinder/signals.h"
#include "cinder/Json.h"

typedef ci::signals::Signal<void(float, bool)> NewStatus;



class SignalsManager {
    
public:
    
    // singleton pattern
    static SignalsManager& Instance(){
        static SignalsManager instance;
        return instance;
    };
    
    NewStatus status;

    
    
private:
    
    // singleton pattern
    SignalsManager(){};
    SignalsManager(SignalsManager const&){};
    SignalsManager& operator=(SignalsManager const&){ return Instance(); };
    
};

