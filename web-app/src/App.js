import './App.css';
import Login from './pages/Login';
import Admin from './pages/Admin';
import firebase from "firebase/app";
import "firebase/auth";
import "firebase/firestore";
import { BrowserRouter as Router, Switch, Route, Link } from 'react-router-dom';
import React from 'react';

const firebaseConfig = {
  apiKey: "Insert Firebase API key here",
  authDomain: "lln-smart-city.firebaseapp.com",
  projectId: "lln-smart-city",
  storageBucket: "lln-smart-city.appspot.com",
  messagingSenderId: "699639001376",
  appId: "1:699639001376:web:4442a17211a6bceac1a745"
};

firebase.initializeApp(firebaseConfig);

function App() {
  const [user, setUser] = React.useState(null)
  React.useEffect(() => firebase.auth().onAuthStateChanged((user) => {
  if (user) {
    setUser(user)
  } else {
    setUser(null)
  }
}))

  return (
    <Router>
      <Switch>
        {user
          ? <Route path="/">
              <Admin />
            </Route>
          : <Route path="/">
              <Login />
            </Route>
        }
      </Switch>
    </Router>
  );
}

export default App;
