#if !defined SAIL_DATA_COLLECTION_H

enum tut_state
{
    ts_sideToSide,
    ts_upToSpeed,
    ts_windChange,
    ts_storm,
};

struct tutorial_data
{
    tut_state tutorialState;
    bool32 movedToSides;
    bool32 upToSpeed;
    bool32 windChanged;

    //recorded in seconds
    r32 timeSpentInState;

    i32 windChangeAmount;
};

#define SAIL_DATA_COLLECTION_H
#endif

