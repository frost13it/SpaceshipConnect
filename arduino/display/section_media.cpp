#include "SpaceshipDisplay.h"

typedef SpaceshipDisplay::Media Media;

void Media::clear() {
    wma(false);
    mp3(false);
    asel(false);
    ta(false);
    tp(false);
    tel(false);
    wireless(false);
    track(false);
    folder(false);
    disk(false);
    dolby(false);
    channel(false);
    news(false);
    st(false);
    scan(false, false, false);
    rpt(false, false, false);
    rdm(false, false);
    for (int i = 1; i < 6; ++i) {
        diskNumber(i, false);
    }
}

void Media::wma(bool value) {
    segments.writeSegment(13, value);
}

void Media::mp3(bool value) {
    segments.writeSegment(173, value);
}

void Media::asel(bool value) {
    segments.writeSegment(39, value);
}

void Media::ta(bool value) {
    segments.writeSegment(92, value);
}

void Media::tp(bool value) {
    segments.writeSegment(172, value);
}

void Media::tel(bool value) {
    segments.writeSegment(257, value);
}

void Media::wireless(bool value) {
    segments.writeSegment(119, value);
}

void Media::track(bool value) {
    segments.writeSegment(157, value);
}

void Media::folder(bool value) {
    segments.writeSegment(284, value);
}

void Media::disk(bool value) {
    segments.writeSegment(199, value);
}

void Media::dolby(bool value) {
    segments.writeSegment(237, value);
}

void Media::channel(bool value) {
    segments.writeSegment(262, value);
}

void Media::news(bool value) {
    segments.writeSegment(287, value);
}

void Media::st(bool value) {
    segments.writeSegment(317, value);
}

void Media::scan(bool d, bool f, bool scan) {
    segments.writeSegment(159, scan);
    segments.writeSegment(158, d);
    segments.writeSegment(238, f);
    segments.writeSegment(318, d || f);
}

void Media::rpt(bool d, bool f, bool rpt) {
    segments.writeSegment(239, rpt);
    segments.writeSegment(315, d);
    segments.writeSegment(310, f);
    segments.writeSegment(300, d || f);
}

void Media::rdm(bool f, bool rdm) {
    segments.writeSegment(319, rdm);
    segments.writeSegment(295, f && rdm);
}

static constexpr uint8_t DISK_NUMBER_SEGMENTS[] = {252, 253, 93, 78, 79};

void Media::diskNumber(uint8_t number, bool value) {
    segments.writeSegment(DISK_NUMBER_SEGMENTS[number - 1], value);
}
