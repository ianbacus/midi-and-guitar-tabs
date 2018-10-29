
let ScoreView = new View();
let ScoreModel = new Model();
let ScoreController = new Controller(ScoreView,ScoreModel);

$( function()
{
    ScoreView.Initialize(
        ScoreController.OnKeyUp,
        ScoreController.OnMouseMove,
        ScoreController.OnHover,
        ScoreController.OnKeyPress,
        ScoreController.OnButtonPress,
    );

});
