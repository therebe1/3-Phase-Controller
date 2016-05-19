/* 
 * File:   ThreePhaseController.cpp
 * Author: Cameron
 * 
 * Created on October 22, 2015, 2:21 AM
 */

#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "ThreePhaseController.h"
#include "MLX90363.h"
#include "ThreePhaseDriver.h"
#include "Debug.h"
#include "Interpreter.h"
#include "Predictor.h"

// u4 ThreePhaseController::Predictor::drivePhase;
// u2 ThreePhaseController::Predictor::lastMagPha;
// s2 ThreePhaseController::Predictor::driveVelocity;
bool ThreePhaseController::isForwardTorque;
u1 ThreePhaseController::magRoll;

void TIMER4_OVF_vect() {
 ThreePhaseController::isr();
}

void ThreePhaseController::isr() {
 u1 static mlx = 1;

 // Scale phase to output range
 u2 outputPhase = Predictor::predict();

 // Offset from current angle by 90deg for max torque
 if (isForwardTorque) outputPhase += ThreePhaseDriver::StepsPerCycle / 4;
 else                 outputPhase -= ThreePhaseDriver::StepsPerCycle / 4;
 
 // Fix outputPhase range
 if (outputPhase > ThreePhaseDriver::StepsPerCycle) {
  // Fix it
  if (isForwardTorque) outputPhase -= ThreePhaseDriver::StepsPerCycle;
  else                 outputPhase += ThreePhaseDriver::StepsPerCycle;
 }
 
 // Update driver outputs
 ThreePhaseDriver::advanceTo(outputPhase);
 
 // Don't continue if we're not done counting down
 if (--mlx)
  return;
 
 MLX90363::startTransmitting();
 
 mlx = cyclesPWMPerMLX;
}

//size of table in program memory
u2 constexpr loop = 4097;

/**
 * 12-bit lookup table for magnetometer Alpha value to Phase value
 */
static const u2 AlphaToPhaseLookup[loop] PROGMEM = {
 // Table generated with python on calibration branch
 342,340,339,337,336,334,332,331,329,327,324,323,320,318,317,315,313,312,310,309,307,305,304,302,300,298,297,296,294,292,291,288,
 287,285,282,281,278,276,274,272,270,267,265,263,259,256,251,248,247,245,243,242,241,239,238,237,236,235,234,233,233,232,231,230,
 230,229,229,228,227,226,225,224,223,221,220,219,217,216,214,212,211,208,206,204,201,199,196,194,192,189,186,184,181,179,177,175,
 173,171,169,167,165,163,161,159,157,155,153,151,149,146,144,142,141,139,138,135,134,132,129,126,124,122,120,119,117,115,114,112,
 111,110,108,107,106,105,105,104,104,103,103,103,102,102,102,101,101,101,100,100, 99, 98, 97, 96, 95, 95, 93, 93, 91, 89, 89, 88,
  86, 84, 82, 80, 78, 76, 75, 72, 70, 68, 66, 64, 62, 59, 57, 55, 53, 51, 49, 48, 45, 44, 42, 40, 39, 38, 35, 35, 33, 32, 30, 29,
  27, 26, 25, 23, 22, 20, 18, 17, 15, 14, 12, 10,  9,  8,  6,  6,  2,763,762,761,760,759,757,757,755,755,754,753,752,752,751,750,
 750,749,749,749,749,748,748,748,747,746,746,745,745,744,743,742,742,741,741,740,739,737,736,735,734,733,731,730,728,727,726,724,
 723,720,719,717,715,713,712,709,708,706,704,703,701,699,698,696,695,693,692,690,689,688,686,686,684,683,682,680,679,678,678,676,
 675,674,674,673,671,670,669,668,667,666,665,664,663,661,659,658,657,656,654,653,651,650,649,647,646,644,642,641,639,637,636,634,
 633,631,631,629,629,627,626,625,624,624,622,621,619,619,617,617,616,615,614,613,612,611,610,609,609,608,607,606,605,604,603,602,
 601,600,599,598,596,595,594,592,591,590,589,587,586,584,582,581,579,579,576,575,574,573,571,570,569,568,567,565,564,563,562,561,
 560,560,559,557,557,556,555,554,553,553,552,552,552,551,550,549,549,548,547,546,546,544,544,543,542,541,540,540,539,538,537,536,
 535,534,532,532,530,529,527,526,524,523,521,520,519,518,512,511,506,505,504,503,501,500,498,497,495,494,492,491,490,488,487,486,
 485,483,483,481,480,479,479,477,477,476,474,473,472,472,472,471,470,469,468,467,466,465,464,463,463,462,461,459,459,458,457,456,
 456,455,454,453,452,452,450,450,448,448,447,445,445,444,443,442,440,440,439,438,437,436,435,434,434,434,433,432,432,431,431,431,
 430,430,429,429,429,428,428,428,427,427,426,426,425,425,425,425,424,424,423,423,422,422,422,421,420,419,418,417,417,416,415,413,
 412,411,411,409,408,407,405,404,402,400,398,398,396,395,393,392,391,389,388,386,383,382,380,378,377,376,375,374,373,371,370,369,
 368,367,366,364,363,362,361,360,360,359,358,358,357,357,356,356,355,354,353,352,351,351,351,350,349,348,347,346,346,345,344,343,
 342,342,340,340,338,338,336,336,335,333,333,331,331,330,329,328,326,325,324,323,323,322,321,320,320,319,318,317,317,316,315,315,
 314,314,314,313,312,311,311,311,309,309,307,307,306,305,304,304,303,302,301,300,299,298,296,296,294,293,291,291,288,287,285,284,
 282,281,279,277,275,273,271,270,268,266,264,263,260,257,254,250,249,248,246,245,243,242,241,239,238,237,236,234,234,232,231,230,
 229,229,227,226,225,224,224,223,222,221,220,220,218,218,217,216,216,214,213,212,211,210,208,207,206,205,203,202,200,199,197,197,
 194,193,192,190,189,187,186,184,182,181,179,178,176,176,174,173,172,170,169,167,166,165,163,162,161,159,158,157,155,154,153,152,
 150,149,146,145,144,142,141,140,139,137,135,133,131,129,127,124,122,121,119,117,115,114,112,111,109,107,106,104,103,102,101,101,
 100, 99, 98, 97, 96, 96, 95, 95, 94, 93, 93, 92, 92, 91, 91, 90, 90, 90, 89, 89, 88, 88, 87, 87, 86, 84, 83, 83, 82, 81, 79, 78,
  77, 75, 73, 72, 70, 68, 67, 65, 63, 61, 59, 57, 56, 54, 52, 51, 49, 47, 45, 44, 42, 41, 39, 38, 36, 35, 33, 32, 30, 29, 27, 26,
  25, 22, 21, 19, 18, 16, 15, 13, 11, 10,  8,  7,  6,  3,765,762,761,760,759,758,756,755,754,753,752,752,752,752,751,751,751,750,
 750,750,750,750,750,749,749,749,748,748,747,747,746,745,744,743,742,741,740,739,737,736,733,732,730,728,726,723,722,719,717,715,
 712,709,708,705,703,700,698,696,694,692,690,688,686,684,682,680,679,677,676,674,673,671,669,668,666,664,663,661,659,657,655,654,
 652,650,648,647,645,642,640,639,636,634,633,632,632,631,630,629,628,628,626,626,625,624,623,622,620,619,618,617,615,614,612,610,
 609,607,604,602,600,598,595,593,590,587,584,582,579,576,574,571,569,567,564,562,560,558,555,554,552,550,548,546,544,542,541,539,
 537,535,534,532,530,528,526,524,523,520,519,516,512,507,504,503,501,498,497,494,492,490,488,487,485,483,481,480,478,477,475,474,
 472,472,470,469,467,467,465,464,463,461,460,458,457,456,454,453,451,449,448,446,444,442,440,438,436,434,432,431,430,428,427,426,
 424,424,423,422,421,420,419,418,417,417,416,415,414,413,412,411,411,410,409,408,407,405,404,402,400,398,396,395,393,391,389,386,
 383,380,378,377,375,373,372,370,369,367,366,364,362,361,360,359,358,357,356,355,353,352,351,350,349,348,346,345,344,342,341,339,
 338,337,335,334,333,331,330,328,326,325,323,322,321,320,319,318,317,317,316,315,314,314,314,313,313,312,311,311,311,310,309,308,
 308,306,306,304,304,302,301,300,298,297,295,294,291,289,288,285,283,281,278,276,274,271,269,266,264,263,258,256,250,248,247,245,
 244,242,240,238,237,235,234,232,230,229,228,227,225,224,223,221,221,219,218,216,216,214,213,211,210,208,207,206,205,203,201,200,
 199,198,197,195,194,192,191,189,188,187,186,184,183,182,181,179,179,178,177,176,175,174,173,172,171,170,169,167,167,165,164,162,
 161,160,158,157,155,154,152,150,149,146,144,142,141,139,138,135,133,131,128,126,124,121,119,118,116,114,112,111,109,107,105,104,
 103,101,100, 99, 97, 96, 95, 94, 92, 91, 90, 90, 89, 88, 87, 87, 86, 84, 83, 83, 82, 82, 81, 80, 79, 79, 78, 77, 76, 74, 74, 73,
  72, 71, 70, 69, 67, 66, 65, 64, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 51, 50, 49, 48, 47, 45, 45, 43, 42, 42, 41, 40, 39, 38,
  37, 35, 35, 34, 32, 31, 30, 29, 28, 27, 25, 24, 23, 22, 21, 20, 18, 17, 16, 15, 13, 12, 10,  9,  8,  7,  6,  4,  2,762,762,761,
 760,758,756,755,753,752,751,750,749,748,747,746,745,744,744,743,742,742,742,742,742,741,741,741,741,741,740,740,740,739,739,739,
 739,739,738,738,738,737,737,736,736,735,735,735,734,734,733,732,732,731,730,729,728,728,727,726,725,723,723,721,720,719,717,716,
 714,713,712,710,709,708,706,705,703,702,701,699,698,696,696,694,693,691,691,689,688,687,686,685,684,682,681,680,679,678,677,675,
 675,673,672,671,670,669,668,666,665,664,663,661,661,659,657,656,655,654,653,652,651,649,649,647,646,645,644,643,642,641,639,638,
 637,636,635,635,634,633,632,632,632,632,631,631,631,630,630,629,629,629,628,628,628,628,627,627,627,626,626,626,625,625,625,624,
 624,623,622,622,621,620,619,618,617,617,615,615,613,613,611,610,609,607,606,605,603,601,601,598,597,595,593,592,591,589,587,585,
 583,582,579,578,576,575,573,571,570,569,567,565,564,563,560,560,557,556,555,553,552,551,550,548,547,545,544,543,541,540,539,537,
 537,535,534,532,531,530,528,527,526,524,523,522,520,520,518,518,513,512,509,506,505,504,503,502,501,500,498,497,496,495,494,493,
 492,491,490,489,489,488,487,486,486,485,485,485,484,483,482,482,481,480,480,479,479,477,477,476,475,474,473,472,472,471,470,469,
 467,467,465,464,463,462,460,459,457,456,454,453,451,450,448,446,445,443,441,440,438,436,435,433,432,431,429,428,426,426,424,423,
 421,421,419,418,417,415,415,413,412,411,410,409,408,407,406,405,403,402,400,399,398,398,397,395,394,393,392,391,390,388,387,385,
 383,381,380,378,377,375,375,373,373,372,370,370,368,367,366,365,363,363,361,361,360,359,358,358,357,357,356,355,354,353,352,351,
 351,350,349,347,347,345,345,344,342,342,340,339,338,336,336,334,333,331,329,328,326,324,323,321,320,318,317,315,314,313,312,311,
 310,309,308,307,306,306,305,304,304,303,302,301,300,300,299,298,298,297,297,296,295,294,293,292,291,289,288,287,285,284,282,281,
 278,277,274,272,271,268,266,264,262,258,256,251,249,248,246,244,242,241,238,237,236,234,232,231,229,227,226,224,224,222,221,219,
 218,216,215,214,211,210,208,206,205,203,201,200,198,197,195,194,192,191,189,188,187,185,184,183,182,181,180,179,178,178,177,176,
 175,174,174,173,172,170,169,168,167,166,165,163,162,160,158,156,154,153,150,147,145,142,140,138,135,133,129,126,123,121,118,116,
 114,112,110,107,105,103,101, 99, 97, 96, 94, 93, 91, 89, 89, 87, 85, 84, 83, 82, 80, 79, 78, 77, 75, 74, 73, 71, 69, 67, 66, 65,
  63, 61, 60, 59, 57, 56, 55, 54, 52, 51, 50, 49, 48, 47, 45, 44, 43, 42, 41, 40, 38, 37, 35, 33, 32, 29, 28, 26, 24, 21, 19, 16,
  14, 11,  9,  7,  3,764,761,759,757,754,753,750,748,747,745,743,742,741,740,739,737,736,735,734,733,732,730,729,728,728,727,726,
 724,723,722,720,719,718,716,715,713,711,709,707,706,704,701,700,698,696,695,693,691,689,688,686,685,683,681,679,678,676,674,672,
 671,668,666,665,662,660,657,655,653,651,649,646,644,641,639,636,634,632,631,629,627,626,625,624,623,621,620,619,618,618,617,616,
 616,615,615,614,613,613,612,611,610,609,608,607,605,604,602,601,599,597,595,593,591,589,587,585,582,579,578,575,572,571,568,566,
 563,561,559,557,554,553,551,549,547,544,543,541,539,537,535,533,531,529,527,525,523,521,519,518,512,511,506,504,503,502,500,498,
 497,496,494,494,493,492,491,490,489,489,488,488,487,486,486,486,485,485,485,484,483,482,482,481,480,479,478,477,477,475,474,473,
 472,471,470,468,466,464,463,460,459,456,454,452,450,449,447,445,443,441,439,437,435,433,432,430,428,427,425,423,422,420,418,417,
 415,413,411,410,409,408,406,405,403,401,399,398,397,395,394,393,392,391,389,387,385,383,380,379,377,376,376,375,374,373,373,372,
 372,371,371,370,370,369,369,368,367,367,366,365,365,364,363,362,361,361,360,359,358,358,357,356,355,353,352,351,349,348,347,345,
 344,342,341,339,337,336,333,332,330,328,326,325,323,321,319,318,316,315,313,312,311,309,308,307,305,304,303,301,300,299,298,296,
 296,294,293,292,291,290,288,288,286,285,284,283,282,281,280,278,277,276,275,273,272,271,270,268,267,265,264,263,262,260,256,251,
 250,249,248,247,245,244,243,242,241,240,238,237,236,236,234,234,232,231,230,229,229,227,227,226,224,224,222,221,221,219,218,217,
 216,216,214,214,212,211,209,208,207,205,204,202,201,200,198,197,196,194,193,192,190,189,188,186,185,184,182,181,179,179,178,176,
 175,174,174,173,171,170,169,169,168,167,166,166,166,165,164,163,162,162,162,161,161,160,159,159,158,157,156,156,155,155,154,153,
 152,151,150,149,147,145,144,143,142,141,139,139,137,136,135,133,132,129,127,126,124,122,121,119,119,117,115,114,113,112,110,108,
 107,106,104,103,102,101,100, 99, 97, 97, 95, 95, 94, 93, 91, 91, 90, 89, 88, 88, 87, 86, 85, 84, 83, 82, 81, 80, 80, 79, 79, 77,
  77, 75, 75, 74, 73, 72, 70, 70, 69, 67, 67, 66, 65, 64, 63, 62, 61, 61, 60, 59, 59, 58, 58, 57, 56, 56, 56, 55, 55, 54, 54, 54,
  53, 52, 52, 52, 51, 51, 50, 50, 50, 49, 49, 48, 48, 47, 47, 47, 46, 45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 40, 38, 38, 37, 36,
  35, 34, 33, 32, 31, 30, 28, 27, 26, 25, 23, 22, 20, 19, 18, 16, 15, 13, 11,  9,  8,  7,  6,  4,  2,763,762,761,760,760,758,757,
 756,754,753,753,751,750,749,748,747,746,745,744,743,742,741,741,740,739,738,737,736,735,735,734,733,732,732,730,729,729,728,728,
 727,726,725,725,723,723,721,721,719,719,718,717,715,714,713,712,711,710,709,708,708,707,706,704,703,703,702,701,700,700,699,699,
 698,697,696,695,695,694,693,693,691,690,690,689,688,688,687,686,685,684,683,682,680,679,678,677,676,675,673,672,670,669,667,665,
 664,663,661,659,657,656,654,653,651,649,647,646,644,642,640,638,636,634,633,631,630,629,627,626,625,624,623,621,620,619,618,617,
 615,615,614,613,612,611,610,609,608,607,607,606,605,604,603,602,601,600,599,598,597,596,595,594,592,592,590,589,587,586,585,584,
 582,580,579,578,576,574,573,572,570,569,568,566,565,563,562,560,559,557,556,554,554,552,551,550,549,547,546,544,543,542,540,539,
 538,536,534,533,532,530,528,527,525,523,521,520,518,515,512,508,505,504,503,501,499,497,495,494,493,490,490,487,486,485,484,483,
 481,481,479,479,478,477,476,476,476,475,475,474,474,473,473,473,473,472,472,471,470,470,470,469,468,467,466,466,464,464,462,461,
 459,458,456,455,453,452,450,448,447,445,444,442,440,438,436,434,432,431,429,427,425,424,423,421,419,418,416,415,413,411,410,409,
 407,405,404,402,401,398,398,396,395,393,392,391,389,387,385,383,381,380,377,376,376,375,375,374,374,373,373,373,372,372,372,372,
 371,371,371,370,370,370,369,369,368,367,366,365,365,364,363,362,361,360,359,358,357,356,354,353,351,349,347,345,342,340,338,336,
 334,331,329,327,324,322,320,318,316,314,312,310,308,306,304,302,300,298,297,295,294,291,290,288,285,284,282,280,278,276,274,272,
 271,268,266,264,263,261,256,254,250,249,248,247,246,245,244,243,242,241,240,239,238,237,236,234,233,231,231,229,228,227,224,223,
 221,218,217,214,212,210,208,205,202,200,198,195,193,190,188,185,182,180,177,175,173,171,169,167,165,163,161,159,157,156,154,153,
 151,149,146,145,142,142,140,139,137,135,132,130,127,124,122,120,118,117,114,112,111,108,106,104,103,101, 99, 97, 96, 95, 94, 92,
  91, 90, 89, 87, 86, 84, 83, 82, 81, 80, 78, 77, 75, 74, 72, 70, 69, 67, 65, 63, 61, 59, 58, 56, 55, 53, 52, 50, 49, 48, 46, 45,
  44, 42, 41, 41, 40, 40, 39, 38, 37, 37, 36, 35, 34, 34, 33, 32, 31, 29, 29, 27, 26, 25, 23, 22, 20, 18, 16, 14, 12, 10,  8,  6,
   3,764,762,760,759,757,755,753,752,750,749,747,746,745,743,742,741,739,738,737,735,734,733,731,730,729,728,727,726,725,723,722,
 721,719,718,717,715,714,712,711,710,709,707,706,705,704,703,702,701,700,700,699,699,698,697,696,695,695,694,693,692,691,690,690,
 688,688,687,686,684,683,682,680,679,678,676,675,673,671,669,667,665,663,661,658,656,654,652,650,647,645,642,640,637,635,633,632,
 630,629,626,626,624,622,621,619,617,616,615,613,612,611,609,608,607,605,604,603,601,600,598,596,595,594,592,591,589,587,586,584,
 582,581,579,578,576,575,574,572,571,570,569,568,567,566,565,563,562,562,560,559,558,557,556,555,554,553,552,551,550,549,547,547,
 545,543,541,540,539,537,535,533,531,529,527,525,523,521,520,518,512,511,506,504,503,501,499,497,495,493,491,490,487,486,485,483,
 481,480,478,477,476,475,474,473,472,472,471,470,469,469,468,467,466,465,465,464,463,463,462,461,459,459,458,458,456,455,455,453,
 453,452,451,450,448,447,446,445,444,443,441,440,439,438,437,435,434,433,432,431,430,429,428,426,425,424,423,422,421,421,419,418,
 417,416,415,414,413,412,411,410,409,408,407,406,405,403,401,400,399,398,396,395,394,393,392,391,389,388,386,383,381,380,378,377,
 375,375,374,373,372,371,370,369,369,368,366,365,365,364,363,362,362,362,361,361,361,361,360,361,360,360,360,359,359,359,359,358,
 358,358,358,357,357,357,356,356,355,355,354,354,353,352,352,351,351,350,349,347,346,345,345,344,342,341,340,338,337,336,334,333,
 331,330,328,327,325,324,323,321,320,318,317,316,315,314,313,312,311,309,308,307,305,304,303,301,301,299,298,297,296,295,294,293,
 291,290,289,288,287,285,284,283,282,281,280,278,278,276,275,274,273,272,271,270,268,267,266,264,264,263,262,262,258,256,253,251,
 250,250,249,248,248,247,247,247,246,245,244,244,244,244,243,243,243,242,242,241,241,240,240,240,239,238,238,237,237,237,236,235,
 235,234,233,232,231,231,230,229,228,227,226,224,224,223,221,220,219,217,216,215,214,212,211,209,207,205,204,202,200,199,197,196,
 194,192,190,189,187,185,183,181,180,178,176,175,174,172,170,169,167,167,165,164,162,161,160,159,157,156,155,153,152,151,149,148,
 146,144,143,142,141,140,139,138,136,135,133,132,130,128,126,124,123,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,
 106,105,105,104,103,102,102,102,101,101,100,100, 99, 99, 98, 97, 96, 96, 95, 95, 94, 93, 92, 91, 90, 90, 89, 88, 87, 86, 85, 83,
  82, 81, 80, 78, 77, 75, 73, 72, 70, 69, 67, 66, 64, 62, 60, 59, 58, 56, 56, 54, 52, 51, 49, 48, 46, 45, 44, 42, 41, 40, 39, 38,
  37, 35, 35, 33, 32, 32, 30, 29, 28, 27, 27, 25, 25, 23, 22, 21, 20, 19, 18, 17, 16, 15, 13, 11, 10,  9,  8,  7,  6,  5,  2,763,
 762,761,760,759,757,756,755,753,753,751,750,749,748,747,746,745,744,743,742,742,741,740,739,738,738,737,736,735,735,734,733,732,
 731,730,729,728,728,727,726,725,723,722,721,720,718,717,715,713,712,711,709,708,707,705,703,702,701,699,698,696,695,694,693,692,
 691,690,690,689,688,687,687,686,686,685,684,683,682,681,681,680,679,678,678,677,675,674,673,672,671,669,668,666,665,663,661,658,
 657,654,653,651,649,647,644,642,640,638,636,634,632,631,629,628,626,625,623,622,620,618,616,615,614,612,611,609,608,606,605,603,
 601,601,599,597,596,594,592,591,589,587,586,584,582,581,579,577,576,574,572,571,570,568,568,566,565,564,563,562,562,561,560,559,
 558,557,556,555,554,553,552,551,550,548,547,545,543,541,539,537,535,533,530,527,525,522,520,517,512,509,504,502,500,496,494,491,
 489,487,485,483,481,479,477,476,475,473,472,471,470,468,467,465,464,463,461,460,458,457,456,454,453,451,449,447,445,444,442,440,
 439,438,436,435,434,433,432,430,430,429,428,427,426,425,424,423,422,421,419,418,416,415,413,411,410,408,405,403,401,398,395,393,
 391,388,385,382,379,377,375,373,371,370,367,365,363,362,360,359,358,357,357,356,354,353,352,352,351,350,349,348,347,346,345,343,
 342,
};

/**
 * Converts the magnetometer to a phase value
 * @param alpha 14-bit value from magnetometer
 * @return phase value (0 - 0x2ff inclusive)
 */
inline static u2 lookupAlphaToPhase(u2 alpha) {
 // Make sure we're working with a 14-bit number
 alpha &= 0x7fff;
 
 //divide alpha by 4 to get a feasible table size
 alpha >>= 2;
 
 //if somehow we get a magnetomiter reading larger then in calibration
 //circle around to begining
 if (alpha > loop){
  alpha -= loop;
 }

 // Read the phase number word from the calculated place in the lookup table
 return pgm_read_word(&AlphaToPhaseLookup[alpha]);
}

void ThreePhaseController::init() {
 MLX90363::init();
 ThreePhaseDriver::init();
 ThreePhaseDriver::setAmplitude(0);
 
 MLX90363::prepareGET1Message(MLX90363::MessageType::Alpha);

 // Enable Timer4 Overflow Interrupt
 TIMSK4 = 1 << TOIE4;
 
 magRoll = MLX90363::getRoll();

 // Get two new readings to get started
 while (!MLX90363::hasNewData(magRoll));
 while (!MLX90363::hasNewData(magRoll));
 
 Predictor::init();
}

void ThreePhaseController::setTorque(const Torque t) {
 ATOMIC_BLOCK(ATOMIC_FORCEON) {
  isForwardTorque = t.forward;
  ThreePhaseDriver::setAmplitude(t.amplitude);
 }
}

bool ThreePhaseController::updateDriver() {
 if (!MLX90363::hasNewData(magRoll)) return false;
 
 // We can always grab the latest Alpha value safely here
 auto const alpha = MLX90363::getAlpha();
 auto const magPha = lookupAlphaToPhase(alpha);
 
 Predictor::freshPhase(magPha);

 return true;
}

// predictor = new PositionPredictor(time_between_mag_updates)
// u2 ThreePhaseController::Predictor::predict(){

//  u4 ph = drivePhase;
//  ph += driveVelocity;
 
//  const bool forward = driveVelocity > 0;
 
//  const u4 MAX = ThreePhaseDriver::StepsPerCycle << drivePhaseValueShift;
 
//  // Check if ph(ase) value is out of range
//  if (ph > MAX) {
//   // Fix it
//   if (forward)
//    ph -= MAX;
//   else
//    ph += MAX;
//  }
 
//  // Store new drivePhase
//  drivePhase = ph;
 
//  // Adjust output for velocity lag
//  ph += driveVelocity * driveVelocityPhaseAdvance;
 
//   // Check if ph(ase) value is out of range again
//  if (ph > MAX) {
//   // Fix it
//   if (forward) ph -= MAX;
//   else         ph += MAX;
//  }
 
//  return (ph >> drivePhaseValueShift);
// }

// void ThreePhaseController::Predictor::freshPhase(u2 phase){

 
//  auto tempVelocity = driveVelocity;
 
//  const s2 measuredPhaseChange = phase - lastMagPha; 
 
//  tempVelocity = nextVelocity(tempVelocity, measuredPhaseChange);
 
//  ATOMIC_BLOCK(ATOMIC_FORCEON) {
//   driveVelocity = tempVelocity;
//   drivePhase = u4(phase) << drivePhaseValueShift;
//  }
 
//  static u1 tick = 0;

//  Debug::SOUT
//          << Debug::Printer::Special::Start
//          << tick++
//          << phase
//          << Debug::Printer::Special::End;

//  // Save the most recent magnetic position
//  lastMagPha = phase;
 
// }

// s4 ThreePhaseController::Predictor::nextVelocity(tempVelocity, measuredPhaseChange){

//  const s2 predictedPhaseChange = (s4(tempVelocity) * cyclesPWMPerMLX) >> drivePhaseValueShift;

//  //TODO make this actually reflect max acceleration
//  if (measuredPhaseChange > predictedPhaseChange) {
//   tempVelocity++;
//  } else if (measuredPhaseChange < predictedPhaseChange) {
//   tempVelocity--;
//  }

//  return tempVelocity;
// }

// void ThreePhaseController::Predictor::init(){

//  driveVelocity = 0;
//  lastMagPha = lookupAlphaToPhase(MLX90363::getAlpha());
//  drivePhase = lastMagPha << drivePhaseValueShift;
// }
