/*
 * Gait.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: hex
 */
#include"Gait.h"
#include <fstream>
#include <iostream>

namespace RobotHighLevelControl{

PushRecoveryPlanner CGait::onlinePlanner;
EGAIT CGait::m_currentGait[AXIS_NUMBER];
long long int CGait::m_gaitStartTime[AXIS_NUMBER];
int CGait::m_gaitCurrentIndex[AXIS_NUMBER];
EGaitState CGait::m_gaitState[AXIS_NUMBER];
Aris::RT_CONTROL::CMotorData CGait::m_standStillData[AXIS_NUMBER];
Aris::RT_CONTROL::CMotorData CGait::m_commandDataMapped[AXIS_NUMBER];
Aris::RT_CONTROL::CMotorData CGait::m_feedbackDataMapped[AXIS_NUMBER];
int CGait::Gait_iter[AXIS_NUMBER];
int CGait::Gait_iter_count[AXIS_NUMBER];
bool CGait::IsConsFinished[AXIS_NUMBER];
bool CGait::IsHomeStarted[AXIS_NUMBER];
double CGait::m_screwLength[AXIS_NUMBER];
int CGait::m_commandMotorCounts[AXIS_NUMBER];

///////////////////////////////////////////////
int GaitMove[GAIT_MOVE_LEN][GAIT_WIDTH];
int GaitMoveBack[GAIT_MOVEBACK_LEN][GAIT_WIDTH];

int GaitHome2Start[GAIT_HOME2START_LEN][GAIT_WIDTH];

///////////////////////////////////////////////
int GaitMove_Acc[GAIT_ACC_LEN][GAIT_WIDTH];
int GaitMove_Cons[GAIT_CON_LEN][GAIT_WIDTH];
int GaitMove_Dec[GAIT_DEC_LEN][GAIT_WIDTH];

int GaitMoveBack_Acc[GAIT_ACC_LEN][GAIT_WIDTH];
int GaitMoveBack_Cons[GAIT_CON_LEN][GAIT_WIDTH];
int GaitMoveBack_Dec[GAIT_DEC_LEN][GAIT_WIDTH];

int GaitFastMove_Acc[GAIT_FAST_ACC_LEN][GAIT_WIDTH];
int GaitFastMove_Cons[GAIT_FAST_CON_LEN][GAIT_WIDTH];
int GaitFastMove_Dec[GAIT_FAST_DEC_LEN][GAIT_WIDTH];

int GaitFastMoveBack_Acc[GAIT_FAST_ACC_LEN][GAIT_WIDTH];
int GaitFastMoveBack_Cons[GAIT_FAST_CON_LEN][GAIT_WIDTH];
int GaitFastMoveBack_Dec[GAIT_FAST_DEC_LEN][GAIT_WIDTH];

int GaitLegUp[GAIT_LEGUP_LEN][GAIT_WIDTH];

int GaitTurnLeft[GAIT_TURN_LEN][GAIT_WIDTH];

int GaitTurnRight[GAIT_TURN_LEN][GAIT_WIDTH];


void CGait::MapFeedbackDataIn(Aris::RT_CONTROL::CMachineData& p_data )
{
    for(int i=0;i<MOTOR_NUM;i++)
    {

        m_feedbackDataMapped[i].Position=p_data.feedbackData[MapAbsToPhy[i]].Position;
        m_feedbackDataMapped[i].Velocity=p_data.feedbackData[MapAbsToPhy[i]].Velocity;
        m_feedbackDataMapped[i].Torque=p_data.feedbackData[MapAbsToPhy[i]].Torque;
    }
};
void CGait::MapCommandDataOut(Aris::RT_CONTROL::CMachineData& p_data )
{
    for(int i=0;i<MOTOR_NUM;i++)
    {
        p_data.commandData[i].Position=m_commandDataMapped[MapPhyToAbs[i]].Position;
        p_data.commandData[i].Velocity=m_commandDataMapped[MapPhyToAbs[i]].Velocity;
        p_data.commandData[i].Torque=m_commandDataMapped[MapPhyToAbs[i]].Torque;
    }
};

CGait::CGait()
{
    for(int i=1;i<AXIS_NUMBER;i++)
    {
        CGait::m_currentGait[i]=EGAIT::GAIT_NULL;
        CGait::m_gaitState[i]=EGaitState::NONE;
        CGait::IsHomeStarted[i]=false;
        CGait::IsConsFinished[i]=false;
        CGait::Gait_iter[i]=1;
        CGait::Gait_iter_count[i]=0;

    }

};
CGait::~CGait()
{
};

bool CGait::IsGaitFinished()
{
    for(int i=0;i<AXIS_NUMBER;i++)
    {
        if(m_gaitState[i]!=EGaitState::GAIT_STOP)
            return false;
    }
    return true;
};

static std::ifstream fin;
//read file
int CGait::InitGait(Aris::RT_CONTROL::CSysInitParameters& param)
{
    double timemidleg=2;
    double period=2;
    double stepsize=0.3;
    double stepheight=0.04;
    double alpha=0.5;

    int Line_Num;
    int ret;

    double temp;
    std::cout << "Start Reading File" << std::endl;
    std::cout<<"reading file data..."<<std::endl;

    fin.open("../../resource/gait/TL.txt");

    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0;i<GAIT_TURN_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitTurnLeft[i][j]=-(int)temp;
            //cout<<GaitTurnLeft[i][j]<<endl;

        }
    }
    fin.close();

    fin.open("../../resource/gait/TR.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0;i<GAIT_TURN_LEN;i++)
    {
        fin>>Line_Num;


        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitTurnRight[i][j]=-(int)temp;

        }
    }

    fin.close();

    fin.open("../../resource/gait/leg_up.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0;i<GAIT_LEGUP_LEN;i++)
    {
        fin>>Line_Num;


        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitLegUp[i][j]=-(int)temp;

        }
    }

    fin.close();

    fin.open("../../resource/gait/start.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0;i<GAIT_HOME2START_LEN;i++)
    {
        fin>>Line_Num;


        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitHome2Start[i][j]=-(int)temp;
            //	cout<<"gait start "<<GaitHome2Start[i][j]<<endl;

        }

    }

    fin.close();
    /*FOWARD*/
    fin.open("../../resource/gait/acc.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;
    for(int i=0; i<GAIT_ACC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMove_Acc[i][j]=-(int)temp;
        }

    }
    fin.close();


    fin.open("../../resource/gait/cons.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;
    for(int i=0+GAIT_ACC_LEN; i<GAIT_ACC_LEN+GAIT_CON_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMove_Cons[i-GAIT_ACC_LEN][j]=-(int)temp;
        }

    }
    fin.close();
    fin.open("../../resource/gait/dec.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_ACC_LEN+GAIT_CON_LEN; i<GAIT_ACC_LEN+GAIT_CON_LEN+GAIT_DEC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMove_Dec[i-GAIT_ACC_LEN-GAIT_CON_LEN][j]=-(int)temp;
        }

    }
    fin.close();

    /*backward*/
    fin.open("../../resource/gait/back_acc.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0; i<GAIT_ACC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMoveBack_Acc[i][j]=-(int)temp;
        }

    }
    fin.close();


    fin.open("../../resource/gait/back_cons.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_ACC_LEN; i<GAIT_ACC_LEN+GAIT_CON_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMoveBack_Cons[i-GAIT_ACC_LEN][j]=-(int)temp;
        }

    }
    fin.close();
    fin.open("../../resource/gait/back_dec.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_ACC_LEN+GAIT_CON_LEN; i<GAIT_ACC_LEN+GAIT_CON_LEN+GAIT_DEC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitMoveBack_Dec[i-GAIT_ACC_LEN-GAIT_CON_LEN][j]=-(int)temp;
        }

    }
    fin.close();

    /*fast FOWARD*/

    fin.open("../../resource/gait/fast_acc.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0; i<GAIT_FAST_ACC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMove_Acc[i][j]=-(int)temp;
        }
    }
    fin.close();


    fin.open("../../resource/gait/fast_cons.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_FAST_ACC_LEN; i<GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMove_Cons[i-GAIT_FAST_ACC_LEN][j]=-(int)temp;
        }
    }

    fin.close();
    fin.open("../../resource/gait/fast_dec.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN; i<GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN+GAIT_FAST_DEC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMove_Dec[i-GAIT_FAST_ACC_LEN-GAIT_FAST_CON_LEN][j]=-(int)temp;
        }
    }
    fin.close();
    /*fast backward*/

    fin.open("../../resource/gait/fast_back_acc.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0; i<GAIT_FAST_ACC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMoveBack_Acc[i][j]=-(int)temp;
        }
    }
    fin.close();


    fin.open("../../resource/gait/fast_back_const.txt");
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    for(int i=0+GAIT_FAST_ACC_LEN; i<GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMoveBack_Cons[i-GAIT_FAST_ACC_LEN][j]=-(int)temp;
        }
    }

    fin.close();
    if(fin.fail())
        goto OPEN_FILE_FAIL;

    fin.open("../../resource/gait/fast_back_dec.txt");
    for(int i=0+GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN; i<GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN+GAIT_FAST_DEC_LEN;i++)
    {
        fin>>Line_Num;
        for(int j=0;j<GAIT_WIDTH;j++)
        {
            fin>>temp;
            GaitFastMoveBack_Dec[i-GAIT_FAST_ACC_LEN-GAIT_FAST_CON_LEN][j]=-(int)temp;
        }
    }

    fin.close();

    if (onlinePlanner.LoadData() != 0)
        goto OPEN_FILE_FAIL;

    cout << "All files sucessfully read" << endl;
    return 0;

OPEN_FILE_FAIL:
    cerr << "Open file error: " << strerror(errno) << endl;
    return errno;
};

int CGait::RunGait(double timeNow, EGAIT* p_gait,Aris::RT_CONTROL::CMachineData& p_data, double* givenForce)
{
    //rt_printf("operation mode %d\n",p_data.motorsModes[0]);
    MapFeedbackDataIn(p_data);
    for(int i=0;i<AXIS_NUMBER;i++)
    {
        int motorID =MapPhyToAbs[i];
        switch(p_gait[i])
        {
            case GAIT_ONLINE:
                // it should dealt with in a specific place
                break;

            case GAIT_NULL:
                CGait::m_gaitState[i]=EGaitState::GAIT_STOP;

                m_currentGait[i] = p_gait[i];
                m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                m_commandDataMapped[motorID].Position=m_standStillData[motorID].Position;
                m_commandDataMapped[motorID].Velocity=m_standStillData[motorID].Velocity;
                m_commandDataMapped[motorID].Torque=m_standStillData[motorID].Torque;

                //rt_printf("driver %d: GAIT_NULL...\n",i);

                break;
            case GAIT_HOME:

                m_currentGait[i] = p_gait[i];
                m_commandDataMapped[motorID].Position=m_feedbackDataMapped[motorID].Position;
                m_commandDataMapped[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                m_commandDataMapped[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                if(p_data.isMotorHomed[i]==true)
                {
                    m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                    m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                    m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                    p_gait[i]=GAIT_STANDSTILL;
                    rt_printf("driver %d: HOMED\n",i);

                }

                break;

            case GAIT_STANDSTILL:
                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d:   GAIT_STANDSTILL begins\n",i);
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;

                    m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                    m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                    m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                    m_commandDataMapped[motorID].Position=m_standStillData[motorID].Position;
                    m_commandDataMapped[motorID].Velocity=m_standStillData[motorID].Velocity;
                    m_commandDataMapped[motorID].Torque=m_standStillData[motorID].Torque;
                }
                else
                {
                    m_commandDataMapped[motorID].Position=m_standStillData[motorID].Position;
                    m_commandDataMapped[motorID].Velocity=m_standStillData[motorID].Velocity;
                    m_commandDataMapped[motorID].Torque=m_standStillData[motorID].Torque;
                }
                break;
            case GAIT_HOME2START:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_HOME2START begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitHome2Start[0][motorID];

                }
                else
                {
                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);

                    m_commandDataMapped[motorID].Position=GaitHome2Start[m_gaitCurrentIndex[i]][motorID];


                    if(m_gaitCurrentIndex[i]==GAIT_HOME2START_LEN-1)
                    {
                        rt_printf("driver %d:GAIT_HOME2START will transfer to GAIT_STANDSTILL...\n",i);
                        p_gait[i]=GAIT_STANDSTILL;

                        m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                        m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                        m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;
                        m_gaitState[i]=EGaitState::GAIT_STOP;
                    }

                }
                break;
            case GAIT_LEGUP:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_LEGUP begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitLegUp[0][motorID];

                }
                else
                {
                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);

                    m_commandDataMapped[motorID].Position=GaitLegUp[m_gaitCurrentIndex[i]][motorID];


                    if(m_gaitCurrentIndex[i]==GAIT_LEGUP_LEN-1)
                    {
                        rt_printf("driver %d:GAIT_LEGUP will transfer to GAIT_STANDSTILL...\n",i);
                        p_gait[i]=GAIT_STANDSTILL;

                        m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                        m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                        m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;
                        m_gaitState[i]=EGaitState::GAIT_STOP;
                    }

                }
                break;
            case GAIT_TURN_LEFT:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_TURNLEFT begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitTurnLeft[0][motorID];

                }
                else
                {
                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);

                    m_commandDataMapped[motorID].Position=GaitTurnLeft[m_gaitCurrentIndex[i]][motorID];


                    if(m_gaitCurrentIndex[i]==GAIT_TURN_LEN-1)
                    {
                        rt_printf("driver %d:GAIT_TURNLEFT will transfer to GAIT_STANDSTILL...\n",i);
                        p_gait[i]=GAIT_STANDSTILL;

                        m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                        m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                        m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;
                        m_gaitState[i]=EGaitState::GAIT_STOP;
                    }

                }
                break;
            case GAIT_TURN_RIGHT:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_TURNRIGHT begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitTurnRight[0][motorID];

                }
                else
                {
                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);

                    m_commandDataMapped[motorID].Position=GaitTurnRight[m_gaitCurrentIndex[i]][motorID];


                    if(m_gaitCurrentIndex[i]==GAIT_TURN_LEN-1)
                    {
                        rt_printf("driver %d:GAIT_TURNRIGHT will transfer to GAIT_STANDSTILL...\n",i);
                        p_gait[i]=GAIT_STANDSTILL;

                        m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                        m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                        m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;
                        m_gaitState[i]=EGaitState::GAIT_STOP;
                    }

                }
                break;

            case GAIT_MOVE:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_MOVE begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitMove_Acc[0][motorID];

                    rt_printf("driver %d:Begin Accelerating foward...\n",i);
                }
                else
                {

                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);
                    //rt_printf("index%d and Gait_iter_count%d and Gait_iter%d\n",m_gaitCurrentIndex,Gait_iter_count,Gait_iter);



                    //rt_printf("MOVE order:%d\n",GaitMove_Acc[m_gaitCurrentIndex][0]);
                    if(m_gaitCurrentIndex[i]<GAIT_ACC_LEN)
                    {
                        m_commandDataMapped[motorID].Position=GaitMove_Acc[m_gaitCurrentIndex[i]][motorID];
                    }

                    else //if(m_gaitCurrentIndex>=GAIT_ACC_LEN)
                    {
                        if(Gait_iter[i]>=1&&IsConsFinished[i]==false)
                        {


                            m_commandDataMapped[motorID].Position=GaitMove_Cons[(m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN][motorID];

                            if(((m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN)==0)
                            {

                                Gait_iter_count[i]++;

                            }
                            if(((m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN)==GAIT_CON_LEN-1)
                            {

                                Gait_iter[i]--;

                            }
                        }
                        else
                        {
                            IsConsFinished[i]=true;
                            //	rt_printf("GAIT_MOVE just finished CONS stage...\n");
                            //  rt_printf("GAIT_MOVE will be deccelarating...\n");

                            m_commandDataMapped[motorID].Position=GaitMove_Dec[(m_gaitCurrentIndex[i]-GAIT_ACC_LEN-GAIT_CON_LEN*Gait_iter_count[i])][motorID];

                        }


                        if(m_gaitCurrentIndex[i]==GAIT_ACC_LEN+GAIT_CON_LEN*Gait_iter_count[i]+GAIT_DEC_LEN-1)
                        {
                            rt_printf("driver %d: GAIT_MOVE will transfer to GAIT_STANDSTILL...\n",i);

                            p_gait[i]=GAIT_STANDSTILL;


                            m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                            m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                            m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                            //only in this cycle, out side get true from IsGaitFinished()
                            m_gaitState[i]=EGaitState::GAIT_STOP;
                            Gait_iter_count[i]=0;
                            Gait_iter[i]=1;
                            IsConsFinished[i]=false;

                        }
                    }
                }
                break;
            case GAIT_MOVE_BACK:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_MOVE_BACK begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitMoveBack_Acc[0][motorID];

                    rt_printf("driver %d:Begin Accelerating backward...\n",i);
                }
                else
                {

                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);
                    //rt_printf("index%d and Gait_iter_count%d and Gait_iter%d\n",m_gaitCurrentIndex,Gait_iter_count,Gait_iter);

                    //rt_printf("MOVE order:%d\n",GaitMove_Acc[m_gaitCurrentIndex][0]);
                    if(m_gaitCurrentIndex[i]<GAIT_ACC_LEN)
                    {
                        m_commandDataMapped[motorID].Position=GaitMoveBack_Acc[m_gaitCurrentIndex[i]][motorID];
                    }

                    else //if(m_gaitCurrentIndex>=GAIT_ACC_LEN)
                    {
                        if(Gait_iter[i]>=1&&IsConsFinished[i]==false)
                        {


                            m_commandDataMapped[motorID].Position=GaitMoveBack_Cons[(m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN][motorID];

                            if(((m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN)==0)
                            {

                                Gait_iter_count[i]++;

                            }
                            if(((m_gaitCurrentIndex[i]-GAIT_ACC_LEN)%GAIT_CON_LEN)==GAIT_CON_LEN-1)
                            {

                                Gait_iter[i]--;

                            }
                        }
                        else
                        {
                            IsConsFinished[i]=true;
                            //	rt_printf("GAIT_MOVE just finished CONS stage...\n");
                            //  rt_printf("GAIT_MOVE will be deccelarating...\n");

                            m_commandDataMapped[motorID].Position=GaitMoveBack_Dec[(m_gaitCurrentIndex[i]-GAIT_ACC_LEN-GAIT_CON_LEN*Gait_iter_count[i])][motorID];

                        }


                        if(m_gaitCurrentIndex[i]==GAIT_ACC_LEN+GAIT_CON_LEN*Gait_iter_count[i]+GAIT_DEC_LEN-1)
                        {
                            rt_printf("driver %d: GAIT_MOVE_BACK will transfer to GAIT_STANDSTILL...\n",i);

                            p_gait[i]=GAIT_STANDSTILL;


                            m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                            m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                            m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                            //only in this cycle, out side get true from IsGaitFinished()
                            m_gaitState[i]=EGaitState::GAIT_STOP;
                            Gait_iter_count[i]=0;
                            Gait_iter[i]=1;
                            IsConsFinished[i]=false;

                        }
                    }
                }
                break;

            case GAIT_FAST_MOVE:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_FAST_MOVE begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitFastMove_Acc[0][motorID];

                    rt_printf("driver %d:Begin Accelerating foward...\n",i);
                }
                else
                {

                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);
                    //rt_printf("index%d and Gait_iter_count%d and Gait_iter%d\n",m_gaitCurrentIndex,Gait_iter_count,Gait_iter);



                    //rt_printf("MOVE order:%d\n",GaitMove_Acc[m_gaitCurrentIndex][0]);
                    if(m_gaitCurrentIndex[i]<GAIT_FAST_ACC_LEN)
                    {
                        m_commandDataMapped[motorID].Position=GaitFastMove_Acc[m_gaitCurrentIndex[i]][motorID];
                    }

                    else //if(m_gaitCurrentIndex>=GAIT_ACC_LEN)
                    {
                        if(Gait_iter[i]>=1&&IsConsFinished[i]==false)
                        {


                            m_commandDataMapped[motorID].Position=GaitFastMove_Cons[(m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN][motorID];

                            if(((m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN)==0)
                            {

                                Gait_iter_count[i]++;

                            }
                            if(((m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN)==GAIT_FAST_CON_LEN-1)
                            {

                                Gait_iter[i]--;

                            }
                        }
                        else
                        {
                            IsConsFinished[i]=true;
                            //	rt_printf("GAIT_MOVE just finished CONS stage...\n");
                            //  rt_printf("GAIT_MOVE will be deccelarating...\n");

                            m_commandDataMapped[motorID].Position=GaitFastMove_Dec[(m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN-GAIT_FAST_CON_LEN*Gait_iter_count[i])][motorID];

                        }


                        if(m_gaitCurrentIndex[i]==GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN*Gait_iter_count[i]+GAIT_FAST_DEC_LEN-1)
                        {
                            rt_printf("driver %d: GAIT_FAST_MOVE will transfer to GAIT_STANDSTILL...\n",i);

                            p_gait[i]=GAIT_STANDSTILL;


                            m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                            m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                            m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                            //only in this cycle, out side get true from IsGaitFinished()
                            m_gaitState[i]=EGaitState::GAIT_STOP;
                            Gait_iter_count[i]=0;
                            Gait_iter[i]=1;
                            IsConsFinished[i]=false;

                        }
                    }
                }
                break;

            case GAIT_FAST_MOVE_BACK:

                if(p_gait[i]!=m_currentGait[i])
                {
                    rt_printf("driver %d: GAIT_FAST_MOVE_BACK begin\n",i);
                    m_gaitState[i]=EGaitState::GAIT_RUN;
                    m_currentGait[i]=p_gait[i];
                    m_gaitStartTime[i]=p_data.time;
                    m_commandDataMapped[motorID].Position=GaitFastMoveBack_Acc[0][motorID];

                    rt_printf("driver %d:Begin Accelerating backward...\n",i);
                }
                else
                {

                    m_gaitCurrentIndex[i]=(int)(p_data.time-m_gaitStartTime[i]);
                    //rt_printf("index%d and Gait_iter_count%d and Gait_iter%d\n",m_gaitCurrentIndex,Gait_iter_count,Gait_iter);



                    //rt_printf("MOVE order:%d\n",GaitMove_Acc[m_gaitCurrentIndex][0]);
                    if(m_gaitCurrentIndex[i]<GAIT_FAST_ACC_LEN)
                    {
                        m_commandDataMapped[motorID].Position=GaitFastMoveBack_Acc[m_gaitCurrentIndex[i]][motorID];
                    }

                    else //if(m_gaitCurrentIndex>=GAIT_ACC_LEN)
                    {
                        if(Gait_iter[i]>=1&&IsConsFinished[i]==false)
                        {


                            m_commandDataMapped[motorID].Position=GaitFastMoveBack_Cons[(m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN][motorID];

                            if(((m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN)==0)
                            {

                                Gait_iter_count[i]++;

                            }
                            if(((m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN)%GAIT_FAST_CON_LEN)==GAIT_FAST_CON_LEN-1)
                            {

                                Gait_iter[i]--;

                            }
                        }
                        else
                        {
                            IsConsFinished[i]=true;
                            //	rt_printf("GAIT_MOVE just finished CONS stage...\n");
                            //  rt_printf("GAIT_MOVE will be deccelarating...\n");

                            m_commandDataMapped[motorID].Position=GaitFastMoveBack_Dec[(m_gaitCurrentIndex[i]-GAIT_FAST_ACC_LEN-GAIT_FAST_CON_LEN*Gait_iter_count[i])][motorID];

                        }


                        if(m_gaitCurrentIndex[i]==GAIT_FAST_ACC_LEN+GAIT_FAST_CON_LEN*Gait_iter_count[i]+GAIT_FAST_DEC_LEN-1)
                        {
                            rt_printf("driver %d: GAIT_FAST_MOVE_BACK will transfer to GAIT_STANDSTILL...\n",i);

                            p_gait[i]=GAIT_STANDSTILL;


                            m_standStillData[motorID].Position=m_feedbackDataMapped[motorID].Position;
                            m_standStillData[motorID].Velocity=m_feedbackDataMapped[motorID].Velocity;
                            m_standStillData[motorID].Torque=m_feedbackDataMapped[motorID].Torque;

                            //only in this cycle, out side get true from IsGaitFinished()
                            m_gaitState[i]=EGaitState::GAIT_STOP;
                            Gait_iter_count[i]=0;
                            Gait_iter[i]=1;
                            IsConsFinished[i]=false;

                        }
                    }
                }
                break;
        }

    }

    if (onlinePlanner.GetCurrentState() == PushRecoveryPlanner::OGS_ONLINE_DRAG ||
        onlinePlanner.GetCurrentState() == PushRecoveryPlanner::OGS_ONLINE_RETREAT)
    {
        onlinePlanner.GenerateJointTrajectory( timeNow, givenForce, m_screwLength);
        CalculateActualMotorCounts(m_screwLength, m_commandMotorCounts);
        for ( int i = 0; i < AXIS_NUMBER; i++)
        {
            m_commandDataMapped[i].Position = m_commandMotorCounts[i];
        }
        if (fabs(fmod(timeNow, 1.0)) < 2e-3)
        {
            rt_printf("OL cmd: %d\n", m_commandDataMapped[0].Position);
        }
    }
    MapCommandDataOut(p_data);
    //rt_printf("command data pos%d\n",p_data.commandData[0].Position);

    return 0;
};

void CGait::CalculateActualMotorCounts(double *screwLength, int *motorCounts)
{
    int countPerMeter = 350 * 65536; // the counts per meter
    for (int i = 0; i < AXIS_NUMBER; i++)
    {
        motorCounts[i] = countPerMeter * screwLength[i];
    }
}

}
