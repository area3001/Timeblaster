enum BadgelinkMode
{
    eBadgeLinkDisabled,
    eBadgeTransmit,
    eBadgeReceive
};

enum commState
{
    commIdle,
    commTransmitting
};

volatile BadgelinkMode badgelinkMode = eBadgeLinkDisabled;
