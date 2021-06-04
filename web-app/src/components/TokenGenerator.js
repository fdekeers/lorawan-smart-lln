import React from 'react';

import firebase from 'firebase/app';
import 'firebase/auth';
import 'firebase/firestore';

export default function TokenGenerator() {
  const [tokenId, setTokenId] = React.useState('');
  const [newToken, setNewToken] = React.useState('');
  const [error, setError] = React.useState('');

  const handleGenerate = async () => {
    if (tokenId.length > 0) {
      const tokenIdDoc = firebase.firestore().collection('tokens').doc(tokenId);
      const tokenData = await tokenIdDoc.get();
      if (tokenData.exists) {
        setError('Token ID already used');
        return;
      } else {
        setError('');
      }
      fetch(
        'https://lln-smart-city.azurewebsites.net/api/TokenCreation?id=' +
          tokenId
      )
        .then((response) => response.text())
        .then((token) => {
          setNewToken(token);
          tokenIdDoc.set({ token });
        });
    }
  };

  return (
    <>
      <h5>Generate token</h5>
      {error.length > 0 && <p style={{ color: 'red' }}>{error}</p>}
      <div className="row">
        <div className="input-field col s12">
          <input
            id="tokenId"
            type="text"
            value={tokenId}
            onChange={(e) => setTokenId(e.target.value)}
          />
          <label htmlFor="deviceId">Token ID (e.g. my-new-device)</label>
        </div>
        <div className="input-field col s12">
          <button className="btn" onClick={handleGenerate}>
            Generate
          </button>
        </div>
        {newToken && newToken.length > 0 && (
          <div className="col s12">
            Your new token
            <div
              className="green lighten-5"
              style={{
                overflow: 'auto',
                padding: 4,
              }}
            >
              {newToken}
            </div>
          </div>
        )}
      </div>
    </>
  );
}
