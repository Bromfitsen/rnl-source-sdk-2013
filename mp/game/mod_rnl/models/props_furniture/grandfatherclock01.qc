$modelname "props_furniture/grandfatherclock01.mdl"
$scale 1
$cd ".\"
$body "studio" "grandfatherclock01_ref.smd"
$cdmaterials "models\props_furniture"
$staticprop
// 1 sequence
$sequence idle "grandfatherclock01_idle" fps 5 ACT_IDLE 1
$surfaceprop "metal"
$collisionmodel "grandfatherclock01_phys.smd" {
// Mass in kilograms
$concave
$mass 1.0
