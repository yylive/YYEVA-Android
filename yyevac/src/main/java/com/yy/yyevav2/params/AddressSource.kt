package com.yy.yyevav2.params


class AddressSource(val fileAddr: String) : SourceParams() {
    
    override fun toParams(): String {
        return fileAddr
    }
}