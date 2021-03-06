#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    kinect.setup();
    kinect.addDepthGenerator();
    kinect.addImageGenerator();
    kinect.setRegister(false);
    kinect.setMirror(true);
    kinect.addUserGenerator();
    kinect.setMaxNumUsers(3);
    kinect.setSkeletonProfile(XN_SKEL_PROFILE_ALL);
    kinect.start();
    hadUsers = false;
    
    ofSetFrameRate(30);
    
    featExtractor.setup(JOINT_HEAD, JOINT_TORSO, 60);
    
    ofAddListener(kinect.userEvent, this, &ofApp::userEvent);
    ofAddListener(MoDe::ofxMoDeEvent::events, this, &ofApp::mocapExtreme);
    
    ofSetWindowShape(640, 480);
    
    font.load("verdana.ttf", 18);
    
    j = JOINT_RIGHT_HAND;
    f = VELOCITY_MEAN;
    
    ofPtr<ofxMoDeGraph> g1(new ofxMoDeGraph(30, 30, 400, 100, 0, "acc"));
    graphs.push_back(g1);
    ofPtr<ofxMoDeGraph> g2(new ofxMoDeGraph(60, 30, 400, 100, 100, "rms"));
    graphs.push_back(g2);
    ofPtr<ofxMoDeGraph> g3(new ofxMoDeGraph(60, 70, 400, 100, 200, "crest"));
    graphs.push_back(g3);
}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    
    for (int i = 0; i < kinect.getNumTrackedUsers(); i++) {
        ofxOpenNIUser user = kinect.getTrackedUser(i);
        //The following "if" statement is a hard-coded alternative for if(kinect.getUserGenerator().IsNewDataAvailable()), which doesn't work properly in ofxOpenNI
        if (user.getJoint((Joint)0).getWorldPosition() != ofPoint(0,0,0) &&
            user.getJoint((Joint)0).getWorldPosition() != featExtractor.getJoint(0).getDescriptor(MoDe::DESC_POSITION).getCurrent() ) {
            map<int, ofPoint> joints;
            for (int j = 0; j < user.getNumJoints(); j++) {
                joints[j] = user.getJoint((Joint)j).getWorldPosition();
            }
            featExtractor.update(joints);
        }
    }
    
    //This is a trick to reset the user generator if all users are lost
    if (kinect.getNumTrackedUsers()) {
        hadUsers = true;
    } else if (!kinect.getNumTrackedUsers() && hadUsers){   
        hadUsers = false;
        kinect.setPaused(true);
        kinect.removeUserGenerator();
        kinect.addUserGenerator();
        kinect.setPaused(false);
    }
    
    //graphs[0]->addValue(featExtractor.getAcceleration(j).y);
    graphs[0]->addValue(featExtractor.getJoint(j).getDescriptor(MoDe::DESC_ACCELERATION).getCurrent().y);
    graphs[1]->addValue(featExtractor.getJoint(j).getDescriptor(MoDe::DESC_VELOCITY).getCrest().y);
    graphs[2]->addValue(featExtractor.getJoint(j).getDescriptor(MoDe::DESC_ACCELERATION).getCrest().y);
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(ofColor::white);
    float w = ofGetWidth(); //w
    float h = ofGetHeight(); //h
    
    //image
    kinect.drawImage();
    kinect.drawSkeletons();
    
    ostringstream os;
    os << "ofxMoDe example " << endl;
    os << "FPS: " << ofGetFrameRate() << endl;
    ofPoint jointProjectivePosition = kinect.worldToProjective(featExtractor.getJoint(j).getDescriptor(MoDe::DESC_POSITION).getCurrent());
    os << "Quantity of Motion: " << featExtractor.getQom() << endl;
    //os << "Symmetry: " << featExtractor.getSymmetry() << endl;
    os << "Contraction Index: " << featExtractor.getCI() << endl << endl;
    os << "Current joint (left-right to change): " << getJointAsString((Joint)j) << endl;
    os << "Current feature (up-down to change): ";
    switch (f) {
        case VELOCITY_MEAN:
            os << "Velocity magnitude mean" << endl;
            font.drawString(ofToString(featExtractor.getJoint(j).getDescriptor(MoDe::DESC_VELOCITY).getCurrent().y), jointProjectivePosition.x, jointProjectivePosition.y);
            break;
        case ACCELERATION_Y:
            os << "Acceleration along y axis (up-down movement)" << endl;
            //font.drawString(ofToString(featExtractor.getAcceleration(j).y), jointProjectivePosition.x, jointProjectivePosition.y);
            break;
        case RELPOSTOTORSO_X:
            os << "Relative position to torso in x axis" << endl;
            //font.drawString(ofToString(featExtractor.getRelativePositionToTorso(j).x), jointProjectivePosition.x, jointProjectivePosition.y);
            break;
            
        default:
            break;
    }
    
    os << featExtractor.getJoint(JOINT_RIGHT_HAND).getDescriptor(MoDe::DESC_ACCELERATION).getCrest().y;
    
    ofSetColor(0,0,0,100);
    ofDrawRectangle(10, 10, 500, 150);
    ofSetColor(255,255,255);
    ofDrawBitmapString(os.str(), 20, 30);
    
    for (auto graph : graphs)
        graph->draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case OF_KEY_RIGHT:
            if (j < JOINT_COUNT-1) {
                j++;
            }
            break;
        case OF_KEY_LEFT:
            if (j > 0){
                j--;
            }
            break;
        case OF_KEY_UP:
            if (f < RELPOSTOTORSO_X) {
                f++;
            }
            break;
        case OF_KEY_DOWN:
            if (f > 0){
                f--;
            }
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::userEvent(ofxOpenNIUserEvent &event){
    //    if (event.userStatus == USER_TRACKING_STOPPED) {
    //        featExtractor.removeSkeleton(0);
    //    }
}

void ofApp::mocapExtreme(MoDe::ofxMoDeEvent &e){
    if (e.joint == JOINT_RIGHT_HAND && e.feature == MoDe::DESC_ACCELERATION && e.axis == MoDe::MOCAP_Y && e.extremeType == MoDe::EXTREME_TYPE_MAX) {
        cout << "new MAX on right hand with value " << e.value << " in axis " << e.axis << endl;
    }
    else if (e.joint == JOINT_RIGHT_HAND && e.feature == MoDe::DESC_ACCELERATION && e.axis == MoDe::MOCAP_Y && e.extremeType == MoDe::EXTREME_TYPE_MIN) {
        cout << "new MIN on right hand with value " << e.value << " in axis " << e.axis << endl;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}