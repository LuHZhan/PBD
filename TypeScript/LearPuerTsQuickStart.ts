import * as UE from 'ue'
import {$ref, $unref, $set, argv, on, toManualReleaseDelegate, releaseManualReleaseDelegate, blueprint} from 'puerts';

let GameMode = argv.getByName("GameMode") as UE.TSGameMode;
let MyWorld = GameMode.GetWorld();
if(GameMode)
{
    UE.KismetSystemLibrary.PrintString(MyWorld,"脚本中获取到GameMode",true,true,new UE.LinearColor(1.0,0.0,0.0,1.0),5.0,"None");
}