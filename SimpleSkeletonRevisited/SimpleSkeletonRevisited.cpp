/*****************************************************************************
*                                                                            *
*  OpenNI 1.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/

// made from OpenNI Samples/NiSimpleSkeleton

#include <iostream>
#include <iomanip>
#include <signal.h> // interruption with ctrl key + c

#include <XnCppWrapper.h> // only OpenNI header necessary

#define MAX_NUM_USERS 15

using namespace std;

// list of the joints:
// - XN_SKEL_HEAD
// - XN_SKEL_NECK
// - XN_SKEL_TORSO
// - XN_SKEL_WAIST

// - XN_SKEL_LEFT_COLLAR
// - XN_SKEL_LEFT_SHOULDER
// - XN_SKEL_LEFT_ELBOW
// - XN_SKEL_LEFT_WRIST
// - XN_SKEL_LEFT_HAND
// - XN_SKEL_LEFT_FINGERTIP

// - XN_SKEL_RIGHT_COLLAR
// - XN_SKEL_RIGHT_SHOULDER
// - XN_SKEL_RIGHT_ELBOW
// - XN_SKEL_RIGHT_WRIST
// - XN_SKEL_RIGHT_HAND
// - XN_SKEL_RIGHT_FINGERTIP

// - XN_SKEL_LEFT_HIP
// - XN_SKEL_LEFT_KNEE
// - XN_SKEL_LEFT_ANKLE
// - XN_SKEL_LEFT_FOOT

// - XN_SKEL_RIGHT_HIP
// - XN_SKEL_RIGHT_KNEE
// - XN_SKEL_RIGHT_ANKLE
// - XN_SKEL_RIGHT_FOOT


// programm interruption
// https://stackoverflow.com/questions/17766550/ctrl-c-interrupt-event-handling-in-linux
volatile sig_atomic_t interruption_flag = 0;
void interrupt_program(int sig) { // can be called asynchronously
    interruption_flag = 1; // set flag
}


//---------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------
xn::Context g_Context;
xn::ScriptNode g_scriptNode;
xn::UserGenerator g_UserGenerator;

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";

// Callback functions: automatically called by OpenNI

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " New User " << nId << endl;
    
    // New user found
    if (g_bNeedPose) {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    } else {
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " Lost user " << nId << endl;	
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie) {

    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " Pose " << strPose << " detected for user " << nId << endl;
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie) {
    
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " Calibration started for user " << nId << endl;
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie) {
    
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);

    if (eStatus == XN_CALIBRATION_STATUS_OK) {
        // Calibration succeeded
        cout << epochTime << " Calibration complete, start tracking user " << nId << endl;		
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    } else {
    
        // Calibration failed
        cout << epochTime << " Calibration failed for user " << nId << endl;
        
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT) {
            cout << "Manual abort occured, stop attempting to calibrate!" << endl;
            return;
        }

        if (g_bNeedPose) {
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        } else {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}

#define CHECK_RC(nRetVal, what)					                                    \
    if (nRetVal != XN_STATUS_OK) {								                    \
        cout << what << " failed: " << xnGetStatusString(nRetVal) << endl;          \
        return nRetVal;                                                             \
    }



int main() {
    
    // fixing number of digits
    std::cout << std::fixed;
    std::cout << std::setprecision(2);

    // Register signals for program interruption 
    signal(SIGINT, interrupt_program); 

    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    // the main object (context) is initialized from a config file
    std::string fn = "SamplesConfig.xml";
    cout << "Reading config from: " << fn << endl;
    nRetVal = g_Context.InitFromXmlFile(fn.c_str(), g_scriptNode, &errors);

    if (nRetVal == XN_STATUS_NO_NODE_PRESENT) {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        cout << strError << endl;
        return nRetVal;
    } else if (nRetVal != XN_STATUS_OK) {
        cout << "Open failed: " << xnGetStatusString(nRetVal) << endl;
        return nRetVal;
    }

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
    if (nRetVal != XN_STATUS_OK) {
        nRetVal = g_UserGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
        cout << "Supplied user generator doesn't support skeleton" << endl;
        return 1;
    }

    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration()) {
        
        g_bNeedPose = TRUE;

        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)) {
            cout << "Pose required, but not supported" << endl;
            return 1;
        }

        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation joint;

    cout << "Starting to run" << endl;
    if(g_bNeedPose) {
        cout << "Assume calibration pose" << endl;
    }

    // main tracking loop
	while(!interruption_flag) {

        g_Context.WaitOneUpdateAll(g_UserGenerator);

        // print the torso information for the first user already tracking
        nUsers=MAX_NUM_USERS;
        g_UserGenerator.GetUsers(aUsers, nUsers);

        for(XnUInt16 i=0; i<nUsers; i++) {

            if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE) {
                continue;
            }

            // get the joint 3d coordinates
            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_TORSO,joint);
            
            // display coordinates for one joint
            // joint.position is of type XnSkeletonJointPosition
            cout << "user " << aUsers[i] << ": torso at ";
            cout << "(" << joint.position.position.X;
            cout << "," << joint.position.position.Y;
            cout << "," << joint.position.position.Z << ")";
            cout << " -> " << joint.position.fConfidence << endl; // there is also a confidence for each position

            // an orientation is also available for each joint
            XnSkeletonJointOrientation	orientation = joint.orientation;
            // orientation.orientation is a XnMatrix3X3 (3*3 matrix)
            // and there is also a confidence float : joint.orientation.fConfidence: XnConfidence
        }
        
    }

    
    cout << "Cleaning..." << endl;

    // cleaning openNI objects
    g_scriptNode.Release();
    g_UserGenerator.Release();
    g_Context.Release();

    return 0;
}
