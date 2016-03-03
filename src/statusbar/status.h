//
//  status.h
//  ConnectionsWall
//
//  Created by Lab on 9/25/15.
//
//

#include "cinder/Timeline.h"


using StatusRef = std::shared_ptr<class Status>;

class Status {
    
public:
    static StatusRef create(){
        return StatusRef( new Status() );
    }
    void draw();
    bool isDraw = true;

private:
    Status();
    void update(float curStatus, bool draw);
    float mCurStatus;
    bool assetType = false;
    bool isLoading = false;
};
