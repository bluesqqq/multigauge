# Recording format version 1

`RecordingManager` stores files in `/recordings/`. The filesystem creates the
directory when opening the first file. With a UTC start time in years 0001–9999,
the filename is `YYYY-MM-DD_HH-MM-SS.mgr`. Collisions use suffixes `_001`, `_002`,
and so on before `.mgr`. With no start time or a year outside this range, the
manager uses `recording_000001.mgr`, incrementing the number to skip occupied
names. Existing names on the destination are checked after each reboot. The
header preserves the supplied UTC timestamp independently of the filename.

The writer uses `FileSystem::create()` to reserve each filename
atomically, including when an occupied path holds an empty file. Only a name
collision triggers a retry with a new suffix or sequence number. Hosts serialize
recording creation within the destination directory, including cleanup. If a
header write fails, the writer closes and attempts to remove the file it just
created. If cleanup fails, `start()` still reports `StartFailed` and the next
recording skips the occupied name.

All multi-byte fields are little-endian. Records are tightly packed without
alignment or padding. Unsigned integer fields are marked `u`, and signed integer
fields use two's complement. Every CRC is CRC-32/ISO-HDLC (reflected polynomial
`0xEDB88320`, initial value and final XOR `0xFFFFFFFF`). The stored CRC is a `u32`.

## Header

| Field | Encoding |
| --- | --- |
| Magic | Four bytes `4d 47 52 46` (`MGRF`) |
| Version | `u8`, value 1 |
| Flags | `u8`, bit 0 signals a UTC start time. Other bits are zero. |
| Channel count | `u8`, 1–255 |
| UTC start time | `i64`, Unix epoch seconds. Present only when bit 0 is set. |
| Channels | One entry per channel, in index order |
| Header CRC | CRC of all preceding header bytes, starting with the magic |

Each channel entry has a `u16` identifier length, a `u16` base unit label length,
then the identifier bytes followed by the label bytes. Lengths count bytes and
there are no terminators. Identifiers are nonempty and unique by byte comparison.
An empty label denotes a unitless channel. The label describes the canonical base
unit captured with the recording. It is independent of display preferences.
This includes user-defined channels. The writer limits identifiers to 128 bytes
and unit labels to 32 bytes. The wire lengths are still `u16`. Readers can accept
longer strings in otherwise valid v1 files.

## Sample blocks

| Field | Encoding |
| --- | --- |
| Sample count | `u8`, 1–55 |
| First sample time | `u64`, monotonic microseconds since the recording started |
| Samples | The declared number of 9-byte updates |
| Block CRC | CRC from sample count through the last update |

Each update consists of a `u32` microsecond offset from the first sample time,
a `u8` zero-based channel index, and a four-byte IEEE 754 binary32 value in the
channel's base unit. The first offset is zero. Elapsed timestamps are
nondecreasing across blocks, with ties stored in submission order. A new block
starts when the count limit or offset range would be exceeded. The wire value is
always binary32, including on targets with a wider `Measurement`. Converting from
`double` can round values and can underflow very small magnitudes to subnormal
values or zero. Non-finite values and values outside the finite binary32 range
are rejected before any block of the batch is written.

A full block is 508 bytes (9 bytes of framing, 55 × 9 bytes of samples, and a
4-byte CRC). The writer uses a 512-byte scratch buffer and writes only the encoded
bytes. Block offsets depend on the header length and preceding blocks. Physical
sector placement and buffering are backend concerns.

## Finalization and recovery

The final record has a zero sample count (`u8`), the elapsed recording duration
in microseconds (`u64`), and the CRC of those nine bytes. The duration is at least
the last sample time and can be longer. `stop()` writes this record, flushes, and
closes the file.

Successful appends mean the backend accepted the bytes. The host can call
`flush()` periodically to drain backend buffers while leaving the recording
open. A successful flush carries the durability guarantees of the destination
backend. A failed flush ends the recording and reports `FlushFailed`.

Reject incomplete headers, unknown versions or flag bits, and invalid header
CRCs. With a valid header, recover the consecutive complete sample blocks before
EOF or the first truncated or invalid record. Validate each entire block before
using any samples, including its CRC, channel indices, finite values, timestamp
ordering, and overflow when adding offsets to the first sample time. A complete
final record with a valid CRC and duration supplies the actual duration. In its
absence, the last valid sample time is the end of the recoverable timeline.
Recovery reads the original bytes in place. A valid block can only be recovered
if its bytes reached the medium.
