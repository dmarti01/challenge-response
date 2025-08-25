package com.example.nfcsigner

import android.os.Bundle
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.security.KeyPairGenerator
import java.security.KeyStore
import java.security.Signature

class MainActivity : AppCompatActivity() {

    private val keyAlias = "StudentKey"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        ensureKeyExists()

        val challengeInput = findViewById<EditText>(R.id.challengeInput)
        val resultText = findViewById<TextView>(R.id.resultText)
        val signButton = findViewById<Button>(R.id.signButton)

        signButton.setOnClickListener {
            val challengeHex = challengeInput.text.toString()
            if (challengeHex.isEmpty()) {
                resultText.text = "Enter challenge hex"
                return@setOnClickListener
            }

            val challengeBytes = hexStringToByteArray(challengeHex)
            val signature = signChallenge(challengeBytes)
            resultText.text = "Signature:\n$signature"
        }
    }

    private fun ensureKeyExists() {
        val ks = KeyStore.getInstance("AndroidKeyStore")
        ks.load(null)
        if (!ks.containsAlias(keyAlias)) {
            val kpg = KeyPairGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_RSA, "AndroidKeyStore"
            )
            val spec = KeyGenParameterSpec.Builder(
                keyAlias,
                KeyProperties.PURPOSE_SIGN or KeyProperties.PURPOSE_VERIFY
            )
                .setDigests(KeyProperties.DIGEST_SHA256)
                .setSignaturePaddings(KeyProperties.SIGNATURE_PADDING_RSA_PKCS1)
                .build()
            kpg.initialize(spec)
            kpg.generateKeyPair()
        }
    }

    private fun signChallenge(data: ByteArray): String {
        val ks = KeyStore.getInstance("AndroidKeyStore")
        ks.load(null)
        val privateKey = ks.getKey(keyAlias, null) as java.security.PrivateKey
        val signature = Signature.getInstance("SHA256withRSA")
        signature.initSign(privateKey)
        signature.update(data)
        val signed = signature.sign()
        return Base64.encodeToString(signed, Base64.NO_WRAP)
    }

    private fun hexStringToByteArray(s: String): ByteArray {
        val len = s.length
        val data = ByteArray(len / 2)
        var i = 0
        while (i < len) {
            data[i / 2] =
                ((Character.digit(s[i], 16) shl 4) + Character.digit(s[i + 1], 16)).toByte()
            i += 2
        }
        return data
    }
}
