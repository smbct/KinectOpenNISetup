#include "definitions.hpp"

#include <iostream>
#include <GL/freeglut.h>

using namespace std;

#define CHECK_RC(nRetVal, what)					                                    \
    if (nRetVal != XN_STATUS_OK) {								                    \
        cout << what << " failed: " << xnGetStatusString(nRetVal) << endl;          \
        return nRetVal;                                                             \
    }


// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    
    State& state = *static_cast<State*>(glutGetWindowData());
    
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " New User " << nId << endl;
    
    // New user found
    if (state.skel.g_bNeedPose) {
        generator.GetPoseDetectionCap().StartPoseDetection(state.skel.g_strPose, nId);
    } else {
        generator.GetSkeletonCap().RequestCalibration(nId, TRUE);
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

    State& state = *static_cast<State*>(glutGetWindowData());

    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " Pose " << strPose << " detected for user " << nId << endl;
    state.skel.g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    state.skel.g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie) {
    
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    cout << epochTime << " Calibration started for user " << nId << endl;
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie) {
    
    State& state = *static_cast<State*>(glutGetWindowData());

    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);

    if (eStatus == XN_CALIBRATION_STATUS_OK) {
        // Calibration succeeded
        cout << epochTime << " Calibration complete, start tracking user " << nId << endl;		
        state.skel.g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    } else {
    
        // Calibration failed
        cout << epochTime << " Calibration failed for user " << nId << endl;
        
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT) {
            cout << "Manual abort occured, stop attempting to calibrate!" << endl;
            return;
        }

        if (state.skel.g_bNeedPose) {
            state.skel.g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(state.skel.g_strPose, nId);
        } else {
            state.skel.g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}



XnStatus initializeTracking(SkeletonState& skel_state) {

    skel_state.tracking = false;

    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    // the main object (context) is initialized from a config file
    std::string fn = "SamplesConfig.xml";
    cout << "Reading config from: " << fn << endl;
    nRetVal = skel_state.g_Context.InitFromXmlFile(fn.c_str(), skel_state.g_scriptNode, &errors);

    if (nRetVal == XN_STATUS_NO_NODE_PRESENT) {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        cout << strError << endl;
        return nRetVal;
    } else if (nRetVal != XN_STATUS_OK) {
        cout << "Open failed: " << xnGetStatusString(nRetVal) << endl;
        return nRetVal;
    }

    nRetVal = skel_state.g_Context.FindExistingNode(XN_NODE_TYPE_USER, skel_state.g_UserGenerator);
    if (nRetVal != XN_STATUS_OK) {
        nRetVal = skel_state.g_UserGenerator.Create(skel_state.g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!skel_state.g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
        cout << "Supplied user generator doesn't support skeleton" << endl;
        return 1;
    }

    nRetVal = skel_state.g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = skel_state.g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = skel_state.g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (skel_state.g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration()) {
        
        skel_state.g_bNeedPose = TRUE;

        if (!skel_state.g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)) {
            cout << "Pose required, but not supported" << endl;
            return 1;
        }

        nRetVal = skel_state.g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        skel_state.g_UserGenerator.GetSkeletonCap().GetCalibrationPose(skel_state.g_strPose);
    }

    skel_state.g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = skel_state.g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    return XN_STATUS_OK;
}



void updateTracking(SkeletonState& skel_state) {

    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation joint;

    skel_state.g_Context.WaitOneUpdateAll(skel_state.g_UserGenerator);

    nUsers=MAX_NUM_USERS;
    skel_state.g_UserGenerator.GetUsers(aUsers, nUsers);

    size_t user_ind = 0;

    if(skel_state.g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[user_ind]) == FALSE) {
        skel_state.tracking = false;
        return;
    }

    skel_state.tracking = true;

    // fetch all the joints data
    for(auto& joint_type: joints) {
        skel_state.g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[user_ind], joint_type, skel_state.skeleton[joint_type]);
    }

}



void cleanTracking(SkeletonState& skel_state) {
    
    // cleaning openNI objects
    skel_state.g_scriptNode.Release();
    skel_state.g_UserGenerator.Release();
    skel_state.g_Context.Release();
}