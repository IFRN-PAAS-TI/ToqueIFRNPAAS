import HorarioToque from "./horario.js"

export default {
    name: 'Horarios',
    data() {
        return {
            horarios: []
        }
    },
    methods: {
        populate_horarios() {
            if (this.$parent.authenticated) {
                this.horarios.push(new HorarioToque(7, 0, 10, 2, 1))
                this.horarios.push(new HorarioToque(7, 45, 10, 2, 1))
                this.horarios.push(new HorarioToque(8, 20, 10, 2, 1))
            }
        },
        excluirHorario(index) {
            if (index === 0) {
                this.horarios.shift()
            } else {
                this.horarios.splice(index, index)
            }
        },
        editar_horario(index) {
            let horario = this.horarios[index]
            
            console.log(this.$parent.components)
            this.$parent.EditaHorario.horario = horario
            this.$parent.state = this.$parent.TELA_EDICAO
        }
    },
    mounted(){
        this.populate_horarios()
    },
    template: `
    <div id="horarios">
        <a class="logout" href="" v-on:click="this.$parent.deslogar()">Logout</a>
        <h1>Bem vindo {{ this.$parent.username }}</h1>
        <p>Horários:</p>
        <ol>
           <li v-for="(horario, index) in this.horarios">
            {{ horario.hora }}:{{ String(horario.minuto).padStart(2,'0') }}
            <button type="button" v-on:click="this.editar_horario(index)">Editar</button>
            <button type="button" v-on:click="this.excluirHorario(index)">Excluir</button>
           </li>
        </ol>
    </div>`
}
