"""Falls and terrace pools become water, not blue slabs.

Photographs of the district showed flat blue slabs standing in the air over real
stone. They are the concept world's fall sheets and terrace pools: 900 x 120 x 1500
cubes wearing M_PHI_DeepBlue. The project ships a real water material, so those two
sheet calls use it; crystal cones keep the palette blue because that blue is the
vision, not a placeholder.
"""

import io

PATH = "Source/VEIL/Private/World/VEILConceptWorld.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

HELPER = """// FALLS ARE WATER, NOT BLUE BOXES. The falls and the terrace pools were sheets of
// M_PHI_DeepBlue - 9 m by 15 m blue slabs standing in the air, which is exactly what
// the photographs showed. The project ships a real water material: a sheet of that
// reads as falling water, a sheet of flat blue reads as a placeholder.
static UMaterialInterface* VEILFallWater(const UMaterialInterface* Fallback)
{
	static TWeakObjectPtr<UMaterialInterface> Cached;
	if (!Cached.IsValid())
	{
		Cached = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"));
	}
	return Cached.IsValid() ? Cached.Get() : const_cast<UMaterialInterface*>(Fallback);
}

"""

anchor = "void AVEILConceptWorld::BuildWaterAndBridges("
if "VEILFallWater" not in text:
    if text.count(anchor) != 1:
        fails.append("helper anchor count=%d" % text.count(anchor))
    else:
        text = text.replace(anchor, HELPER.replace("\n", NL) + anchor, 1)

SHEET = "FVector(900.f, 120.f, 1500.f), Water,"
if text.count(SHEET) != 2:
    fails.append("sheet count=%d" % text.count(SHEET))
else:
    text = text.replace(SHEET, "FVector(900.f, 120.f, 1500.f), VEILFallWater(Water),")

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (falls use water)" % (PATH, len(src), len(text)))
