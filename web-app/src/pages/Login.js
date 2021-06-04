import React from 'react';

import firebase from "firebase/app";
import "firebase/auth";

export default function Login() {

  const [email, setEmail] = React.useState('');
  const [password, setPassword] = React.useState('');
  const [error, setError] = React.useState('');

  const loginHandler = () => {
    if (email.length > 0 && password.length > 0) {
      firebase.auth().signInWithEmailAndPassword(email, password)
        .then(user => {
          setError('')
          setEmail('');
          setPassword('');
        })
        .catch(error => {
          setError(error.message)
        })
    }
  }

  return (
    <div className="container">
      <div className="row">
        <h1>LLN: a Smart City</h1>
        <div className="col s12">
          <p>You need to connect to make any changes</p>
          {error.length > 0 && <div className="card-panel red">
            <span className="white-text">{error}</span>
          </div>}
        </div>
        <div className="input-field col s12 m5">
          <input id="email" type="email" value={email} className="validate" onChange={(e) => setEmail(e.target.value)} />
          <label htmlFor="email">Email</label>
        </div>
        <div className="input-field col s12 m5">
          <input id="password" type="password" value={password} className="validate" onChange={(e) => setPassword(e.target.value)} />
          <label htmlFor="password">Password</label>
        </div>
        <div className="input-field col s12 m2">
          <button className="btn" onClick={loginHandler}>Connect</button>
        </div>
      </div>
    </div>
  );
}
