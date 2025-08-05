export module calculator:build;

export namespace Build
{
	constexpr bool IsDebug =
#ifdef _DEBUG
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;
}
