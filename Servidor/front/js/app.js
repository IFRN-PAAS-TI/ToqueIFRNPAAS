// import { createApp } from 'vue'
import Horarios from './horarios.js'
import Login from './login.js'
import EditaHorario from "./edita_horario.js"

const { createApp } = Vue

const app = createApp({
    data() {
      return {
        mockAccount: {
          username: "admin",
          password: "admin"
        },
        username: "Admin",
        authenticated: false,
        state: 0,
        MAIN: 0,
        TELA_EDICAO: 1
      }
    },
    methods: {
        deslogar() {
            this.authenticated = false
        },
    },
    components: {
        Horarios,
        Login,
        EditaHorario
    },
    template: `
    <Login v-if="!this.authenticated && this.state === this.MAIN"/>
    <EditaHorario v-else-if="this.authenticated && this.state === this.TELA_EDICAO"/>
    <Horarios v-else/>`
})

app.mount('#app')
