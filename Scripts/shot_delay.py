"""A screenshot on the teleport frame catches the camera inside the body."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


h = P + "VeilGameMode.h"
s = rd(h)
if "RequestShot" not in s:
    a = "\tvoid SanctuaryPass();"
    if s.count(a) != 1:
        raise SystemExit("MISS sanctuary decl")
    s = s.replace(a, a + "\n\tvoid RequestShot();\n\tFString ShotName;\n\tFTimerHandle ShotSettleTimer;", 1)
    wr(h, s)
    print("header: settled-shot timer")

c = P + "VeilGameMode.cpp"
s = rd(c)
a = "\tFScreenshotRequest::RequestScreenshot(FString(Names[Idx]) + TEXT(\".png\"), false, false);"
if a in s:
    s = s.replace(a, "\t// The spring arm needs a frame to extend; a shot taken on the teleport\n"
                     "\t// frame is a picture of the inside of the Runner.\n"
                     "\tShotName = FString(Names[Idx]) + TEXT(\".png\");\n"
                     "\tGetWorldTimerManager().SetTimer(ShotSettleTimer, this, &AVeilGameMode::RequestShot, 0.6f, false);", 1)
    print("shot: waits for the camera to settle")

if "void AVeilGameMode::RequestShot()" not in s:
    s += '''

void AVeilGameMode::RequestShot()
{
	FScreenshotRequest::RequestScreenshot(ShotName, false, false);
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SHOT: %s captured"), *ShotName);
}
'''
    print("shot: RequestShot appended")
wr(c, s)
print("shot delay fix done")
