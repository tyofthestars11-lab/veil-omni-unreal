"""The ATP ledger printed on top of the bar it describes; move it under."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilHud.cpp"
s = open(P, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = ('\t\tBar(X, Y + 56.f, 150.f, 7.f, R->Mito.ATP / FMath::Max(1.f, R->Mito.MaxATP), Gold);\n'
       '\t\tDrawText(R->Mito.Ledger(), GoldDim, X + 158.f, Y + 52.f, nullptr, '
       'static_cast<float>(PHI_Constants::PhiPow(0.35)), false);\n')
new = ('\t\tBar(X, Y + 56.f, 150.f, 7.f, R->Mito.ATP / FMath::Max(1.f, R->Mito.MaxATP), Gold);\n'
       '\t\tDrawText(R->Mito.Ledger(), GoldDim, X, Y + 74.f, nullptr, 0.86f, false);\n')

if old in s:
    s = s.replace(old, new, 1)
    msg = "ledger moved under the ATP bar"
elif "Y + 74.f" in s:
    msg = "already moved"
else:
    msg = "MISS ledger"

open(P, "w", encoding="utf-8", newline="").write(s)
print(msg)
