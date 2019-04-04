#include <iostream>
#include "ros/ros.h"
#include "justina_tools/JustinaHardware.h"
#include "justina_tools/JustinaHRI.h"
//#include "justina_tools/JustinaManip.h"
#include "justina_tools/JustinaNavigation.h"
#include "justina_tools/JustinaKnowledge.h"
#include "justina_tools/JustinaTools.h"
#include "justina_tools/JustinaVision.h"
#include "justina_tools/JustinaTasks.h"
#include "std_msgs/Bool.h"
//#include "string"
#include <sensor_msgs/LaserScan.h>

#define SM_INIT 0
#define SM_RECOG_NAMES 5
#define SM_CONFIRM_NAME 10
#define SM_RECOG_FACES 12
#define SM_ASK_DRINK 20
#define SM_GO_TO_LIVING_ROOM 25
#define SM_CHECK_REACH_LOCATION 30
#define SM_INTRODUCE_TO_JHON 40


#define SM_BRING_GROCERIES_CONF 41
#define SM_BRING_GROCERIES_TAKE 42
#define SM_BAG_DELIVERY 50
#define SM_BAG_DELIVERY_PLACE 60
#define SM_LOOKING_HELP 70
#define SM_GUIDING_ASK 75
#define SM_GUIDING_HELP 80
#define SM_GUIDING_MEMORIZING_OPERATOR 90
#define SM_GUIDING_MEMORIZING_OPERATOR_ELF 91
#define SM_GUIDING_MEMORIZING_OPERATOR_SAY 92
#define SM_GUIDING_PHASE 100
#define SM_GUIDING_STOP 101
#define SM_GUIDING_CAR 102
#define SM_OPEN_DOOR 103
#define SM_FINAL_STATE 110
#define SM_HOKUYO_TEST 1000


#define MAX_ATTEMPTS_RECOG 3
#define MAX_ATTEMPTS_CONF 3

sensor_msgs::LaserScan laser;
std::vector<float> laser_ranges;
bool door_isopen=false;
bool door_loc=false;
int range=0,range_i=0,range_f=0,range_c=0,cont_laser=0;
float laser_l=0;



vision_msgs::VisionFaceObjects recognizeFaces (float timeOut, bool &recognized)
{
    recognized = false;
    int previousSize = 20;
    int sameValue = 0;
    boost::posix_time::ptime curr;
    boost::posix_time::ptime prev = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff;
    vision_msgs::VisionFaceObjects lastRecognizedFaces;

    do
    {
        lastRecognizedFaces = JustinaVision::getFaces();
        
        if(previousSize == 1)
            sameValue ++;
        
        if (sameValue == 3)
            recognized = true;

        else
        {
            previousSize = lastRecognizedFaces.recog_faces.size();
            recognized = false;
        }

        curr = boost::posix_time::second_clock::local_time();
        ros::spinOnce();
    }while(ros::ok() && (curr - prev).total_milliseconds()< timeOut && !recognized);

    std::cout << "recognized:" << recognized << std::endl;
    return lastRecognizedFaces;
}

/*std::string identifyVisitor(float timeOut, bool &recognized){
    recognized = false;
    int previousSize = 20;
    int sameValue = 0;
    int contcarl =0;
    int contdiego = 0;
    int contUnknown =0;
    boost::posix_time::ptime curr;
    boost::posix_time::ptime prev = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff;
    vision_msgs::VisionFaceObjects lastRecognizedFaces;


    do
    {
        lastRecognizedFaces = JustinaVision::getFaces();

        for(int i=0; i<lastRecognizedFaces.recog_faces.size(); i++){
        if(lastRecognizedFaces.recog_faces[i].id=="carl")
            contcarl++;
        else if(lastRecognizedFaces.recog_faces[i].id=="diego")
            contdiego++;
        else if(lastRecognizedFaces.recog_faces[i].id=="Unknown")
            contUnknown++; 
        }
        
        if(previousSize == 1)
            sameValue ++;
        
        if (sameValue == 4)
            recognized = true;

        else
        {
            previousSize = lastRecognizedFaces.recog_faces.size();
            recognized = false;
        }

        curr = boost::posix_time::second_clock::local_time();
        ros::spinOnce();
    }while(ros::ok() && (curr - prev).total_milliseconds()< timeOut && !recognized);

    std::cout << "recognizedFacessize:" << lastRecognizedFaces.recog_faces.size() << std::endl;
    std::cout << "recognized:" << recognized << std::endl;
    std::cout << "cont diego:" << contdiego << std::endl;
    std::cout << "cont carl:" << contcarl << std::endl;
    std::cout << "cont Unknown:" << contUnknown << std::endl;
    
    

    if(contdiego >= contcarl && contdiego >= contUnknown)
        return "diego";
    else if(contcarl >= contdiego && contcarl >= contUnknown)
        return "carl";
    else if(contUnknown >= contdiego && contUnknown >= contcarl)
        return "Unknown";
}
*/


void Callback_laser(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    range=msg->ranges.size();
    range_c=range/2;
    range_i=range_c-(range/10);
    range_f=range_c+(range/10);
    std::cout<<"Range Size: "<< range << "\n ";
    std::cout<<"Range Central: "<< range_c << "\n ";
    std::cout<<"Range Initial: "<< range_i << "\n ";
    std::cout<<"Range Final: "<< range_f << "\n ";

    cont_laser=0;
    laser_l=0;
    for(int i=range_c-(range/10); i < range_c+(range/10); i++)
    {
        if(msg->ranges[i] > 0 && msg->ranges[i] < 4){ 
            laser_l=laser_l+msg->ranges[i];    
            cont_laser++;
        }
    }
    std::cout<<"Laser promedio: "<< laser_l/cont_laser << std::endl;    
    if(laser_l/cont_laser > 2.0){
        door_isopen=true;
    }
    else{
        door_isopen=false;
    }
}

int main(int argc, char** argv)
{
    std::cout << "INITIALIZING receptionist TEST..." << std::endl;
    ros::init(argc, argv, "act_pln");
    ros::NodeHandle n;
    std::cout << system("pacmd set-default-source alsa_input.pci-0000_00_1f.3.analog-stereo") << std::endl;
    JustinaHardware::setNodeHandle(&n);
    JustinaHRI::setNodeHandle(&n);
    JustinaManip::setNodeHandle(&n);
    JustinaNavigation::setNodeHandle(&n);
    JustinaTools::setNodeHandle(&n);
    JustinaVision::setNodeHandle(&n);
    JustinaTasks::setNodeHandle(&n);
    ros::Rate loop(10);

    boost::posix_time::ptime prev;
    boost::posix_time::ptime curr;

    //int c_point=0,i=1;
    bool is_location;
    int nextState = SM_INIT;
    bool fail = false;
    bool success = false;
    float x, y ,z;
    std::stringstream ss;
    std::vector<std::string> tokens;
    int attemptsRecogLoc = 0;
    int attemptsConfLoc = 0;

    std::string lastRecoSpeech;
    std::string say;
    std::string id;
    
    std::string location="coffee_table";
    std::vector<std::string> validCommandsStop;
    std::vector<std::string> validCommandsName;
    std::vector<std::string> validCommandsDrink;

    int minDelayAfterSay = 0;
    int maxDelayAfterSay = 300;

    validCommandsName.push_back("oscar");
    validCommandsName.push_back("diego");
    validCommandsName.push_back("carl");
    validCommandsName.push_back("john");

    validCommandsDrink.push_back("wine");
    validCommandsDrink.push_back("water");
    validCommandsDrink.push_back("tea");
    validCommandsDrink.push_back("beer");


    ros::Subscriber laser_subscriber;
    //laser_subscriber = n.subscribe<sensor_msgs::LaserScan>("/scan", 1, Callback_laser);  

    bool hokuyoRear = false;
    bool userConfirmation = false;
    bool follow_start=false;
    bool alig_to_place=true;
    int cont_z=3;

    vision_msgs::VisionFaceObjects faces;
    vision_msgs::VisionFaceObjects facesreco;
    bool recog =false;
    int contChances=0;
    bool withLeftArm = false;

    bool once=true;
    bool once2=true;

    std::string personOne="none";
    std::string drinkOne="nothing";

    //JustinaHRI::setInputDevice(JustinaHRI::RODE);
    //JustinaHRI::setOutputDevice(JustinaHRI::USB);
    //JustinaHRI::setVolumenInputDevice(JustinaHRI::RODE, 65000);
    JustinaHRI::setVolumenOutputDevice(JustinaHRI::USB, 80000);
    JustinaTools::pdfStart("HelpMeCarry_Plans");
    JustinaHRI::loadGrammarSpeechRecognized("receptionist.xml");//load the grammar

cont_z=0;
    while(ros::ok() && !fail && !success)
    {
        switch(nextState)
        {  
            case SM_INIT:    
                JustinaHRI::loadGrammarSpeechRecognized("receptionist.xml");//load the grammar

                JustinaHRI::enableSpeechRecognized(false);//disable recognized speech
                //JustinaManip::startHdGoTo(0.0, 0.0);
                JustinaHRI::waitAfterSay("Hello human, please put in front of me to see your face.", 3000);

                ros::Duration(1.0).sleep();
                while(!recog && contChances < 3)
                {
                    faces = recognizeFaces (10000, recog);
                    JustinaVision::startFaceRecognition(false);
                    contChances++;
                }

                if(faces.recog_faces.size()==0)
                {
                    JustinaHRI::say("Sorry, I cannot see anybody in front of me");
                    ros::Duration(1.5).sleep();
                    ros::Duration(1.0).sleep();
                    
                }
                else{
                    JustinaHRI::say("I recognized your face");
                    ros::Duration(1.0).sleep();
                }
                cont_z=0;
                //JustinaTasks::detectBagInFront(true, 20000);
                nextState = SM_RECOG_NAMES;

                once = true;
            break;

            

            case SM_RECOG_NAMES:        
                std::cout <<"faces.centroid: "<<faces.recog_faces[0].face_centroid.y <<std::endl;


                if(once)
                {
                    JustinaHRI::waitAfterSay("Welcome, What is your name ?", 500);
                    once=false;
                }
                else
                    JustinaHRI::waitAfterSay("Sorry, Could you repeat your name ?", 500);
                    
                JustinaHRI::enableSpeechRecognized(true);

                if (JustinaHRI::waitForSpecificSentence(validCommandsName,lastRecoSpeech, 5000) )
                {
                    

                    JustinaHRI::enableSpeechRecognized(false);
                    std::cout << lastRecoSpeech << std::endl;
                    personOne = lastRecoSpeech;
                    say.clear();
                    say.append("Is your name ");
                    say.append(personOne);
                    say.append(" ?");
                    JustinaHRI::waitAfterSay(say, 500);
                    JustinaHRI::enableSpeechRecognized(true);
                    JustinaHRI::waitForUserConfirmation(userConfirmation, 5000);
                    JustinaHRI::enableSpeechRecognized(false);
                    if(userConfirmation)
                    {
                        nextState = SM_CONFIRM_NAME;    
                    }
                }
                else
                {    
                    cont_z++;
                }
                JustinaHRI::enableSpeechRecognized(false);
                if(cont_z >= 3)
                {
                    personOne="carl";
                    nextState = SM_CONFIRM_NAME;
                }

            break;
            

            case SM_CONFIRM_NAME:
                             
                ss.str("");
                ss << "Nice to meet you  "; 
                tokens.clear();
                boost::algorithm::split(tokens, personOne, boost::algorithm::is_any_of("_"));
                for(int i = 0; i < tokens.size(); i++)
                    ss << tokens[i] << " ";

                JustinaHRI::waitAfterSay(ss.str(), 500);
                
                faces.recog_faces[0].id = personOne;

                std::cout <<"faces.id: "<<faces.recog_faces[0].id <<std::endl;
                
                



                nextState = SM_RECOG_FACES;
                once = true;
                cont_z = 0;
                std::cout <<"once: "<< once <<std::endl;

            break;
            

            case SM_RECOG_FACES:

                JustinaVision::startFaceRecognition(true);
                while(!recog && contChances < 3)
                {
                    facesreco = JustinaVision::getFaceRecognition ("carl");
                    //JustinaVision::startFaceRecognition(false);
                    contChances++;
                }

                if(facesreco.recog_faces.size()==0)
                {
                    JustinaHRI::say("Sorry, I cannot see anybody in front of me");
                    ros::Duration(1.5).sleep();
                    ros::Duration(1.0).sleep();
                    
                }
                else{
                    JustinaHRI::say("I recognized your face");
                    ros::Duration(1.0).sleep();
                }
                cont_z=0;
  //nextState = SM_ASK_DRINK;
                std::cout<<"size recog"<<facesreco.recog_faces.size()<<std::endl;
                nextState = SM_ASK_DRINK;
            break;







            case SM_ASK_DRINK:          

                    
                if(once2)
                {
                    JustinaHRI::waitAfterSay("What is your favorite drink ?", 500);     
                    once=false;
                    std::cout <<"ENTRE "<<std::endl;
                    once2=false;
                }
                else
                {
                    JustinaHRI::waitAfterSay("Sorry, Could you repeat your favorite drink ?", 500);
                    std::cout <<"NOOO ENTRE "<<std::endl;
                }
                    
                JustinaHRI::enableSpeechRecognized(true);

                if (JustinaHRI::waitForSpecificSentence(validCommandsDrink,lastRecoSpeech, 5000) )
                {
                    std::cout << lastRecoSpeech << std::endl;
                    drinkOne = lastRecoSpeech;
                    say.clear();
                    say.append("did you say  ");
                    say.append(drinkOne);
                    say.append(" ?");
                    JustinaHRI::waitAfterSay(say, 500);
                    JustinaHRI::enableSpeechRecognized(true);
                    JustinaHRI::waitForUserConfirmation(userConfirmation, 5000);
                    JustinaHRI::enableSpeechRecognized(false);
                    if(userConfirmation)
                    {
                        nextState = SM_GO_TO_LIVING_ROOM;    
                    }
                }
                else
                {    
                    cont_z++;
                }

                JustinaHRI::enableSpeechRecognized(false);
                if(cont_z >= 3)
                {
                    drinkOne="beer";
                    nextState = SM_GO_TO_LIVING_ROOM;
                }
 
            break;
            case SM_GO_TO_LIVING_ROOM:
                JustinaHRI::waitAfterSay("Please follow me, I going to introduce you to John", 1000);
               
                if(!JustinaNavigation::getClose(location, 200000))
                    if(!JustinaNavigation::getClose(location, 200000))
                        JustinaNavigation::getClose(location, 200000);
                nextState = SM_CHECK_REACH_LOCATION;
            break;
            
            case SM_CHECK_REACH_LOCATION:

             if(JustinaNavigation::isGlobalGoalReached()){
                    
                    nextState=SM_INTRODUCE_TO_JHON;
                }
            break;
            case SM_INTRODUCE_TO_JHON:
                say.clear();
                say.append("Hi John, he is  ");
                say.append(personOne);
                say.append(" and his favorite drink is ");
                say.append(drinkOne);
                JustinaHRI::waitAfterSay(say, 10000);
                nextState=0;
            break;
           

        }

        ros::spinOnce();
        loop.sleep();
    }



    return 1;
}
