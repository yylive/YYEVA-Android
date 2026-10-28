package com.yy.yyeva.util

import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

object Mp4MetadataReader {

    private const val BOX_MOOV = "moov"
    private const val BOX_UDTA = "udta"
    private const val BOX_META = "meta"
    private const val BOX_KEYS = "keys"
    private const val BOX_ILST = "ilst"
    private const val BOX_DATA = "data"
    private const val BOX_MDTA = "mdta"
    private const val TAG_PREFIX = "yyeffectmp4json[["
    private const val TAG_SUFFIX = "]]yyeffectmp4json"

    fun readMergeInfo(input: InputStream): String? {
        input.use { return readMergeInfo(it.readBytes()) }
    }

    fun readMergeInfo(bytes: ByteArray): String? {
        val moov = findChildBox(bytes, 0, bytes.size, BOX_MOOV) ?: return null
        val udta = findChildBox(bytes, moov.payloadStart, moov.payloadEnd, BOX_UDTA) ?: return null
        val meta = findChildBox(bytes, udta.payloadStart, udta.payloadEnd, BOX_META) ?: return null

        val metaStart = meta.payloadStart + 4 // version + flags
        val keysBox = findChildBox(bytes, metaStart, meta.payloadEnd, BOX_KEYS)
        val ilstBox = findChildBox(bytes, metaStart, meta.payloadEnd, BOX_ILST) ?: return null
        val mergeIndex = keysBox?.let { findMergeInfoIndex(bytes, it) }
        return parseIlst(bytes, ilstBox, mergeIndex)
    }

    private fun parseIlst(bytes: ByteArray, ilstBox: Box, mergeIndex: Int?): String? {
        var cursor = ilstBox.payloadStart
        while (cursor + 8 <= ilstBox.payloadEnd) {
            val item = readBox(bytes, cursor, ilstBox.payloadEnd) ?: break
            val keyIndex = readUInt32(bytes, item.start + 4).toInt()
            if (mergeIndex == null || keyIndex == mergeIndex) {
                val value = parseIlstItem(bytes, item)
                if (!value.isNullOrEmpty()) return value
            }
            cursor += item.size.toInt()
        }
        return null
    }

    private fun parseIlstItem(bytes: ByteArray, itemBox: Box): String? {
        val dataBox = findChildBox(bytes, itemBox.payloadStart, itemBox.payloadEnd, BOX_DATA) ?: return null
        val payloadStart = dataBox.payloadStart + 8
        if (payloadStart > dataBox.payloadEnd) return null
        val raw = bytes.copyOfRange(payloadStart, dataBox.payloadEnd)
        val text = raw.toString(Charsets.ISO_8859_1)
        val start = text.indexOf(TAG_PREFIX)
        val end = text.indexOf(TAG_SUFFIX, start + TAG_PREFIX.length)
        if (start >= 0 && end > start) {
            return text.substring(start + TAG_PREFIX.length, end)
        }
        return null
    }

    private fun findMergeInfoIndex(bytes: ByteArray, keysBox: Box): Int? {
        var cursor = keysBox.payloadStart + 8 // version/flags + entryCount
        val entryCount = readUInt32(bytes, keysBox.payloadStart + 4).toInt()
        for (index in 1..entryCount) {
            val entry = readBox(bytes, cursor, keysBox.payloadEnd) ?: break
            val nameStart = entry.payloadStart
            val nameBytes = bytes.copyOfRange(nameStart, entry.payloadEnd)
            val keyName = nameBytes.toString(Charsets.UTF_8).trimEnd('\u0000')
            if (entry.type == BOX_MDTA && keyName == "mergeinfo") {
                return index
            }
            cursor += entry.size.toInt()
        }
        return null
    }

    private fun findChildBox(bytes: ByteArray, start: Int, end: Int, wantedType: String): Box? {
        var cursor = start
        while (cursor + 8 <= end) {
            val box = readBox(bytes, cursor, end) ?: return null
            if (box.type == wantedType) return box
            cursor += box.size.toInt()
        }
        return null
    }

    private fun readBox(bytes: ByteArray, start: Int, end: Int): Box? {
        if (start + 8 > end) return null
        val size32 = readUInt32(bytes, start)
        val type = readType(bytes, start + 4)
        return when {
            size32 == 0L -> Box(start, (end - start).toLong(), 8, type)
            size32 == 1L -> {
                if (start + 16 > end) return null
                val size64 = readUInt64(bytes, start + 8)
                if (size64 <= 0 || start + size64 > end.toLong()) return null
                Box(start, size64, 16, type)
            }
            else -> {
                if (size32 < 8 || start + size32 > end.toLong()) return null
                Box(start, size32, 8, type)
            }
        }
    }

    private fun readUInt32(bytes: ByteArray, offset: Int): Long {
        return ByteBuffer.wrap(bytes, offset, 4).order(ByteOrder.BIG_ENDIAN).int.toLong() and 0xffffffffL
    }

    private fun readUInt64(bytes: ByteArray, offset: Int): Long {
        return ByteBuffer.wrap(bytes, offset, 8).order(ByteOrder.BIG_ENDIAN).long
    }

    private fun readType(bytes: ByteArray, offset: Int): String {
        return String(bytes, offset, 4, Charsets.ISO_8859_1)
    }

    private data class Box(
        val start: Int,
        val size: Long,
        val headerSize: Int,
        val type: String
    ) {
        val payloadStart: Int get() = start + headerSize
        val payloadEnd: Int get() = (start + size).toInt()
    }
}
