
//console = function() {}

class disabledConsole
{
    constructor()
    {

    }
    log()
    {
        //Do nothing
    }
}
let ScoreView = new View();
let ScoreModel = new Model();
let ScoreController = new Controller(ScoreView,ScoreModel);

ScoreView.console = new disabledConsole();
ScoreModel.console = new disabledConsole();
ScoreController.console = console;


$( function()
{
    ScoreController.Initialize();
    ScoreView.Initialize(
        ScoreController.OnKeyUp,
        ScoreController.OnMouseScroll,
        ScoreController.OnMouseMove, ScoreController.OnMouseClickUp, ScoreController.OnMouseClickDown,
        ScoreController.OnHoverBegin, ScoreController.OnHoverEnd,
        ScoreController.OnButtonPress, ScoreController.OnRadioButtonPress,
    );


});
