class disabledConsole
{
    constructor() {}
    log() {} //Do nothing
};

let ScoreView = new View();
let ScoreModel = new Model();
let ScoreController = new Controller(ScoreView,ScoreModel);

ScoreView.console = new disabledConsole();
ScoreModel.console = new disabledConsole();
ScoreController.console = new disabledConsole();


// ScoreView.console = console;
// ScoreModel.console = console;
// ScoreController.console = console;


$( function()
{
    ScoreView.Initialize(
        ScoreController.OnKeyUp,
        ScoreController.OnMouseScroll,
        ScoreController.OnMouseMove, ScoreController.OnMouseClickUp, ScoreController.OnMouseClickDown,
        ScoreController.OnHoverBegin, ScoreController.OnHoverEnd,
        ScoreController.OnButtonPress, ScoreController.OnRadioButtonPress,
    );
    ScoreController.Initialize();
});
