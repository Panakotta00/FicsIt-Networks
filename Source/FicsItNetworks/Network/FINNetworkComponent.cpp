#include "FINNetworkComponent.h"

bool IFINNetworkComponent::HasNickByNick(FString nick, FString has) const {
	TSet<FString> has_nicks;
	do {
		FString n = "";
		if (!has.Split(" ", &n, &has)) {
			has_nicks.Add(has);
			has = "";
			break;
		}
		if (n.Len() > 0) has_nicks.Add(n);
	} while (has.Len() > 0);

	do {
		int32 pos = nick.Find(" ");
		FString n = "";
		if (!nick.Split(" ", &n, &nick)) {
			n = nick;
			nick = "";
		}
		if (n.Len() > 0 && !has_nicks.Contains(n)) return false;
	} while (nick.Len() > 0);

	return true;
}
