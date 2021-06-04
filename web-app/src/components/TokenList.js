import React from 'react';

import firebase from 'firebase/app';
import 'firebase/auth';
import 'firebase/firestore';

import Swal from 'sweetalert2';

export default function TokenList() {
  const [tokens, setTokens] = React.useState([]);

  React.useEffect(() => {
    firebase
      .firestore()
      .collection('tokens')
      .onSnapshot((snapshot) => {
        if (snapshot.empty) {
          setTokens([]);
          return;
        }
        const tmpDocs = [];
        snapshot.docs.forEach((doc) =>
          tmpDocs.push({ id: doc.id, ...doc.data() })
        );
        setTokens(tmpDocs);
      });
  }, []);

  const handleRevokeToken = (tokenId) => {
    Swal.fire({
      title: 'Are you sure?',
      text: "You won't be able to revert this!",
      icon: 'warning',
      showCancelButton: true,
      confirmButtonColor: '#3085d6',
      cancelButtonColor: '#d33',
      confirmButtonText: 'Yes, revoke it!',
    }).then(async (result) => {
      if (result.isConfirmed) {
        await firebase.firestore().collection('tokens').doc(tokenId).delete();
        Swal.fire('Deleted!', 'Your token has been revoked.', 'success');
      }
    });
  };

  return (
    <>
      <h5>Active tokens</h5>
      <table>
        <thead>
          <tr>
            <th>ID</th>
            <th></th>
            <th>Token</th>
            <th></th>
          </tr>
        </thead>

        <tbody>
          {tokens.map((token) => (
            <tr>
              <td>{token.id}</td>
              <td>
                <button
                  className="btn-flat"
                  onClick={() => navigator.clipboard.writeText(token.token)}
                >
                  <i className="material-icons">content_copy</i>
                </button>
              </td>
              <td>
                <input value={token.token} readOnly disabled />
              </td>
              <td>
                <button
                  className="btn-flat"
                  onClick={() => handleRevokeToken(token.id)}
                >
                  <i className="material-icons red-text">delete</i>
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </>
  );
}
