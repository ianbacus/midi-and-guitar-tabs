
console.log = function() {}

let ScoreView = new View();
let ScoreModel = new Model();
let ScoreController = new Controller(ScoreView,ScoreModel);


$( function()
{
    console.log("begin");
    ScoreController.Initialize();
    ScoreView.Initialize(
        ScoreController.OnKeyUp,
        ScoreController.OnMouseScroll,
        ScoreController.OnMouseMove, ScoreController.OnMouseClickUp, ScoreController.OnMouseClickDown,
        ScoreController.OnHoverBegin, ScoreController.OnHoverEnd,
        ScoreController.OnButtonPress, ScoreController.OnRadioButtonPress,
    );


});
