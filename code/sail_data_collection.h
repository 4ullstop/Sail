#if !defined SAIL_DATA_COLLECTION_H




enum tut_state
{
    ts_sideToSide,
    ts_upToSpeed,
    ts_windChange,
    ts_storm,
};

enum ui_state
{
    us_empty,
    us_bumpers,
    us_sticks,
};

enum tutorial_ui_state
{
    tus_start,
    tus_moveLeft,
    tus_rotateClockwise,
    tus_moveRight,
    tus_rotateCounterClockwise,
    tus_complete,
};

struct tutorial_data
{
    tut_state tutorialState;
    bool32 movedToSides;
    bool32 upToSpeed;
    bool32 windChanged;
    bool32 stormStarted;

    //recorded in seconds
    r32 timeSpentInState;

    i32 windChangeAmount;

    ui_state uiState;
    tutorial_ui_state tutorialUIState;
    bool32 uiTutorialComplete;
};

#define SAIL_DATA_COLLECTION_H
#endif

