#include "iodevice.h"

using namespace Z80;

IODevice::IODevice()
:	m_cpu(nullptr)
{}

IODevice::~IODevice() = default;
