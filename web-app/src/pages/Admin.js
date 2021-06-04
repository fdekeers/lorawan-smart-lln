import React from 'react';
import firebase from 'firebase/app';
import 'firebase/auth';
import TokenGenerator from '../components/TokenGenerator';
import TokenList from '../components/TokenList';

export default function Admin() {
  return (
    <div className="container">
      <div className="row">
        <div className="col s12" style={{ position: 'relative' }}>
          <h1>Admin</h1>
          <button
            style={{ position: 'absolute', right: 0, top: 36 }}
            className="btn red"
            onClick={() => firebase.auth().signOut()}
          >
            Log out
          </button>
        </div>
        <div className="col s12 m6">
          <TokenGenerator />
        </div>
        <div className="col s12 m6">
          <TokenList />
        </div>
      </div>
    </div>
  );
}
